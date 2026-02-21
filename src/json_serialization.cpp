#include "sys-sage.hpp"
#include <nlohmann/json.hpp>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <stddef.h>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

using namespace sys_sage;
using json = nlohmann::json;

static void DumpAttributes(const json &obj) { /* TODO */ }
static void LoadAttributes(const json &obj) { /* TODO */ }

static void CollectComponentsInSubtree(const Component *root, std::unordered_set<const Component *> &components)
{
    components.insert(root);

    for (auto child : root->GetChildren())
        CollectComponentsInSubtree(child, components);
}

void sys_sage::to_json(nlohmann::json &obj, const Component &component)
{
    component.ToJson(obj);
}

void sys_sage::to_json(nlohmann::json &obj, const Component *component)
{
    component->ToJson(obj);
}

void sys_sage::from_json(const nlohmann::json &obj, Component &component)
{
    component.FromJson(obj);
}

void sys_sage::from_json(const nlohmann::json &obj, Component *&component)
{
    std::string componentType = obj["type"].get<std::string>();
    if (componentType == "GenericComponent") {
        component = new Component;
    } else if (componentType == "HW_Thread") {
        component = new Thread;
    } else if (componentType == "Core") {
        component = new Core;
    } else if (componentType == "Cache") {
        component = new Cache;
    } else if (componentType == "Subdivision") {
        component = new Subdivision;
    } else if (componentType == "NUMA") {
        component = new Numa;
    } else if (componentType == "Chip") {
        component = new Chip;
    } else if (componentType == "Memory") {
        component = new Memory;
    } else if (componentType == "Storage") {
        component = new Storage;
    } else if (componentType == "Node") {
        component = new Node;
    } else if (componentType == "QuantumBackend") {
        component = new QuantumBackend;
    } else if (componentType == "AtomSite") {
        component = new AtomSite;
    } else if (componentType == "Qubit") {
        component = new Qubit;
    } else if (componentType == "Topology") {
        component = new Topology;
    } else {
        std::cerr << "error: invalid component type '" << componentType << "'\n";
        component = nullptr;
        return;
    }

    if (component->FromJson(obj)) {
        delete component;
        component = nullptr;
    }
}

void sys_sage::to_json(nlohmann::json &obj, const Relation &relation)
{
    relation.ToJson(obj);
}

void sys_sage::to_json(nlohmann::json &obj, const Relation *relation)
{
    relation->ToJson(obj);
}

template <>
Relation *sys_sage::GetRelation<Relation *>(const nlohmann::json &obj, Component *root)
{
    Relation *relation;

    std::string relationType = obj["type"].get<std::string>();
    if (relationType == "Relation") {
        relation = new Relation;
    } else if (relationType == "DataPath") {
        relation = new DataPath;
    } else if (relationType == "QuantumGate") {
        relation = new QuantumGate;
    } else if (relationType == "CouplingMap") {
        relation = new CouplingMap;
    } else {
        std::cerr << "error: invalid relation type '" << relationType << "'\n";
        return nullptr;
    }

    if (relation->FromJson(obj, root)) {
        delete relation;
        return nullptr;
    }

    return relation;
}

int sys_sage::DumpJson(const Component *component, const std::filesystem::path &path)
{
    std::unordered_set<const Component *> componentsInSubtree;
    CollectComponentsInSubtree(component, componentsInSubtree);
    std::vector<json> relationSubgraph;

    for (auto component : componentsInSubtree) {
        for (auto relationType : RelationType::RelationTypeList) {
            std::vector<Relation *> relations = component->GetRelationsByType(relationType);
            for (auto relation : relations) {
                // print each relation once
                if (relation->GetComponent(0) != component)
                    continue;

                bool inSubtree = true;
                for (auto rComponent : relation->GetComponents()) {
                    // only collect relations that connect components of the subtree
                    if (componentsInSubtree.count(rComponent) == 0) {
                        inSubtree = false;
                        break;
                    }
                }

                if (inSubtree)
                  relationSubgraph.push_back(relation);
            }
        }
    }

    json output = {
        { "componentTree", component }
    };
    if (relationSubgraph.size() > 0)
        output["relationGraph"] = std::move(relationSubgraph);

    if (path.empty()) {
        std::cout << output.dump(4) << std::endl;
        return 0;
    }

    std::filesystem::path file = path / "topo.json";
    std::ofstream stream ( file );
    if (!stream) {
        std::cerr << "Failed to write to '" << file << "'\n";
        return 1;
    }
    stream << output.dump(4) << std::endl;

    return 0;
}

Component *sys_sage::LoadJson(const std::filesystem::path &path)
{
    std::ifstream stream ( path );
    if (!stream)
        return nullptr;

    const json input = json::parse(stream);
    stream.close();

    auto root = input["componentTree"].get<Component *>();
    if (root == nullptr)
        return nullptr;

    if (auto it = input.find("relationGraph"); it != input.end()) {
        for (auto &relation : *it) {
            if (GetRelation<Relation *>(relation, root)) { // failed to parse a relation
                delete root;
                return nullptr;
            }
        }
    }

    return root;
}

void sys_sage::Component::ToJson(json &obj) const
{
    obj = json{
        { "type", GetComponentTypeStr() },
        { "id", id }
    };

    DumpAttributes(obj);

    if (children.size() > 0) {
        std::vector<json> jsonChildren ( children.size() );

        for (size_t i = 0; i < jsonChildren.size(); i++)
            // this recursive call chain might be very memory heavy
            children[i]->ToJson(jsonChildren[i]);

      obj["children"] = jsonChildren;
    }
}

int sys_sage::Component::FromJson(const nlohmann::json &obj)
{
    obj["id"].get_to<int>(id);

    LoadAttributes(obj);
    
    if (auto it = obj.find("children"); it != obj.end()) {
        for (auto &childJson : *it) {
            auto child = childJson.get<Component *>();
            if (child == nullptr)
                return 1;

            child->SetParent(this);
            InsertChild(child);
        }
    }

    return 0;
}

void sys_sage::Relation::ToJson(json &obj) const
{
    obj = json{
        { "type", GetTypeStr() },
        { "category", category },
        { "id", id },
        { "ordered", ordered }
    };

    // to get a unique identifier for the component, we take the ComponentTypeStr() and
    // append the ID of the component to it, like so: "<ComponentTypeStr()>-<ComponentID>"

    std::vector<std::string> uniqueIdentifiers (components.size());
    for (size_t i = 0; i < uniqueIdentifiers.size(); i++)
        uniqueIdentifiers[i] = components[i]->GetComponentTypeStr() + ("-" + std::to_string(components[i]->GetId()));

    obj["components"] = uniqueIdentifiers;

    DumpAttributes(obj);
}

int sys_sage::Relation::FromJson(const nlohmann::json &obj, Component *root)
{
    components.clear();

    for (auto &component : obj["components"]) {
        auto uniqueIdentifier = component.get<std::string>();
        const char *start = uniqueIdentifier.data();
        const char *end = uniqueIdentifier.data() + uniqueIdentifier.size();
        size_t pos = uniqueIdentifier.find('-');
        if (pos == std::string::npos) {
            std::cerr << "error: failed to parse component vector. Expected format is '<ComponentType>-<ComponentID>', e.g. 'Core-0'\n";
            return 1;
        }
        std::string_view componentTypeStr (start, pos);
        int componentId;
        auto [ptr, ec] = std::from_chars(start + pos + 1, end, componentId);
        if (ptr != end || ec != std::errc()) {
            std::cerr << "error: failed to parse component ID\n";
            return 1;
        }

        ComponentType::type componentType;
        if (componentTypeStr == "GenericComponent") {
            componentType = ComponentType::Generic;
        } else if (componentTypeStr == "HW_Thread") {
            componentType = ComponentType::Thread;
        } else if (componentTypeStr == "Core") {
            componentType = ComponentType::Core;
        } else if (componentTypeStr == "Cache") {
            componentType = ComponentType::Cache;
        } else if (componentTypeStr == "Subdivision") {
            componentType = ComponentType::Subdivision;
        } else if (componentTypeStr == "NUMA") {
            componentType = ComponentType::Numa;
        } else if (componentTypeStr == "Chip") {
            componentType = ComponentType::Chip;
        } else if (componentTypeStr == "Memory") {
            componentType = ComponentType::Memory;
        } else if (componentTypeStr == "Storage") {
            componentType = ComponentType::Storage;
        } else if (componentTypeStr == "Node") {
            componentType = ComponentType::Node;
        } else if (componentTypeStr == "QuantumBackend") {
            componentType = ComponentType::QuantumBackend;
        } else if (componentTypeStr == "AtomSite") {
            componentType = ComponentType::AtomSite;
        } else if (componentTypeStr == "Qubit") {
            componentType = ComponentType::Qubit;
        } else if (componentTypeStr == "Topology") {
            componentType = ComponentType::Topology;
        } else {
            std::cerr << "error: invalid component type '" << componentTypeStr << "'\n";
            return 1;
        }

        auto comp = root->GetDescendantById(componentId, componentType);
        if (comp == nullptr) {
            std::cerr << "error: could not find component of type '" << componentTypeStr << "' with ID '" << componentId << "' in the component tree\n";
            return 1;
        }

        components.push_back(comp);
    }

    obj["category"].get_to<RelationCategory::type>(category);
    obj["id"].get_to<int>(id);
    obj["ordered"].get_to<bool>(ordered);

    LoadAttributes(obj);

    return 0;
}

void sys_sage::AtomSite::ToJson(json &obj) const
{
    QuantumBackend::ToJson(obj);

    obj["SiteProperties"] = {
        { "nRows", properties.nRows },
        { "nColumns", properties.nColumns },
        { "nAods", properties.nAods },
        { "nAodIntermediateLevels", properties.nAodIntermediateLevels },
        { "nAodCoordinates", properties.nAodCoordinates },
        { "interQubitDistance", properties.interQubitDistance },
        { "interactionRadius", properties.interactionRadius },
        { "blockingFactor", properties.blockingFactor }
    };
}

int sys_sage::AtomSite::FromJson(const json &obj)
{
    int rval = QuantumBackend::FromJson(obj);
    if (rval)
        return 1;

    obj["SiteProperties"]["nRows"].get_to<int>(properties.nRows);
    obj["SiteProperties"]["nColumns"].get_to<int>(properties.nColumns);
    obj["SiteProperties"]["nAods"].get_to<int>(properties.nAods);
    obj["SiteProperties"]["nAodIntermediateLevels"].get_to<int>(properties.nAodIntermediateLevels);
    obj["SiteProperties"]["nAodCoordinates"].get_to<int>(properties.nAodCoordinates);
    obj["SiteProperties"]["interQubitDistance"].get_to<double>(properties.interQubitDistance);
    obj["SiteProperties"]["interactionRadius"].get_to<double>(properties.interactionRadius);
    obj["SiteProperties"]["blockingFactor"].get_to<double>(properties.blockingFactor);

    return 0;
}

void sys_sage::Cache::ToJson(json &obj) const
{
    Component::ToJson(obj);

    obj["cacheType"] = cache_type;
    if (cache_size > 0)
        obj["cacheSize"] = cache_size;
    if (cache_associativity_ways > 0)
        obj["cacheAssociativityWays"] = cache_associativity_ways;
    if (cache_line_size > 0)
        obj["cacheLineSize"] = cache_line_size;
}

int sys_sage::Cache::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;

    obj["cacheType"].get_to<std::string>(cache_type);
    if (auto it = obj.find("cacheSize"); it != obj.end())
        it->get_to<long long>(cache_size);
    if (auto it = obj.find("cacheAssociativityWays"); it != obj.end())
        it->get_to<int>(cache_associativity_ways);
    if (auto it = obj.find("cacheLineSize"); it != obj.end())
        it->get_to<int>(cache_line_size);

    return 0;
}

void sys_sage::Chip::ToJson(json &obj) const
{
    Component::ToJson(obj);

    obj["chipType"] = type;
    if (!vendor.empty())
        obj["vendor"] = vendor;
    if (!model.empty())
        obj["model"] = model;
}

int sys_sage::Chip::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;

    obj["chipType"].get_to<ChipType::type>(type);
    if (auto it = obj.find("vendor"); it != obj.end())
        it->get_to<std::string>(vendor);
    if (auto it = obj.find("model"); it != obj.end())
        it->get_to<std::string>(model);

    return 0;
}

void sys_sage::Core::ToJson(json &obj) const
{
    Component::ToJson(obj);

#ifdef PROC_CPUINFO
    obj["frequency"] = freq;
#endif
}

int sys_sage::Core::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;

#ifdef PROC_CPUINFO
    if (auto it = obj.find("frequency"); it != obj.end())
      it->get_to<double>(freq);
#endif

    return 0;
}

void sys_sage::CouplingMap::ToJson(json &obj) const
{
    Relation::ToJson(obj);

    obj["fidelity"] = fidelity;
}

int sys_sage::CouplingMap::FromJson(const json &obj, Component *root)
{
    int rval = Relation::FromJson(obj, root);
    if (rval)
        return 1;

    obj["fidelity"].get_to<double>(fidelity);

    return 0;
}

void sys_sage::DataPath::ToJson(json &obj) const
{
    Relation::ToJson(obj);

    obj["dataPathType"] = dp_type;
    obj["bandwidth"] = bw;
    obj["latency"] = latency;
}

int sys_sage::DataPath::FromJson(const json &obj, Component *root)
{
    int rval = Relation::FromJson(obj, root);
    if (rval)
        return 1;

    obj["dataPathType"].get_to<DataPathType::type>(dp_type);
    obj["bandwidth"].get_to<double>(bw);
    obj["latency"].get_to<double>(latency);

    return 0;
}

void sys_sage::Memory::ToJson(json &obj) const
{
    Component::ToJson(obj);

    obj["volatile"] = is_volatile;
    if (size > 0)
        obj["size"] = size;
}

int sys_sage::Memory::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;
    
    obj["volatile"].get_to<bool>(is_volatile);

    if (auto it = obj.find("size"); it != obj.end())
        it->get_to<long long>(size);

    return 0;
}

void sys_sage::Node::ToJson(json &obj) const
{
    Component::ToJson(obj);
}

int sys_sage::Node::FromJson(const json &obj)
{
    return Component::FromJson(obj);
}

void sys_sage::Numa::ToJson(json &obj) const
{
    Subdivision::ToJson(obj);

    if (size > 0)
        obj["size"] = size;
}

int sys_sage::Numa::FromJson(const json &obj)
{
    int rval = Subdivision::FromJson(obj);
    if (rval)
        return 1;

    if (auto it = obj.find("size"); it != obj.end())
        it->get_to<long long>(size);

    return 0;
}

void sys_sage::QuantumBackend::ToJson(json &obj) const
{
    Component::ToJson(obj);

    obj["numQubits"] = num_qubits;
}

int sys_sage::QuantumBackend::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;

    obj["numQubits"].get_to<int>(num_qubits);

    return 0;
}

void sys_sage::QuantumGate::ToJson(json &obj) const
{
    Relation::ToJson(obj);

    obj["gateSize"] = gate_size;
    obj["name"] = name;
    obj["gateLength"] = gate_length;
    obj["gateType"] = gate_type;
    obj["fidelity"] = fidelity;
    obj["unitary"] = unitary;
}

int sys_sage::QuantumGate::FromJson(const json &obj, Component *root)
{
    int rval = Relation::FromJson(obj, root);
    if (rval)
        return 1;

    obj["gateSize"].get_to<size_t>(gate_size);
    obj["name"].get_to<std::string>(name);
    obj["gateLength"].get_to<int>(gate_length);
    obj["gateType"].get_to<QuantumGateType::type>(gate_type);
    obj["fidelity"].get_to<double>(fidelity);
    obj["unitary"].get_to<std::string>(unitary);

    return 0;
}

void sys_sage::Qubit::ToJson(json &obj) const
{
    Component::ToJson(obj);

    obj["q1Fidelity"] = q1_fidelity;
    obj["t1"] = t1;
    obj["t2"] = t2;
    obj["readoutFidelity"] = readout_fidelity;
    obj["readoutLength"] = readout_length;
}

int sys_sage::Qubit::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;

    obj["q1Fidelity"].get_to<double>(q1_fidelity);
    obj["t1"].get_to<double>(t1);
    obj["t2"].get_to<double>(t2);
    obj["readoutFidelity"].get_to<double>(readout_fidelity);
    obj["readoutLength"].get_to<double>(readout_length);

    return 0;
}

void sys_sage::Storage::ToJson(json &obj) const
{
    Component::ToJson(obj);

    if (size > 0)
        obj["size"] = size;
}

int sys_sage::Storage::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;

    if (auto it = obj.find("size"); it != obj.end())
        it->get_to<long long>(size);

    return 0;
}

void sys_sage::Subdivision::ToJson(json &obj) const
{
    Component::ToJson(obj);

    obj["subdivisionType"] = type;
}

int sys_sage::Subdivision::FromJson(const json &obj)
{
    int rval = Component::FromJson(obj);
    if (rval)
        return 1;

    obj["subdivisionType"].get_to<SubdivisionType::type>(type);

    return 0;
}

void sys_sage::Thread::ToJson(json &obj) const
{
    Component::ToJson(obj);
}

int sys_sage::Thread::FromJson(const json &obj)
{
    return Component::FromJson(obj);
}

void sys_sage::Topology::ToJson(json &obj) const
{
    Component::ToJson(obj);
}

int sys_sage::Topology::FromJson(const json &obj)
{
    return Component::FromJson(obj);
}
