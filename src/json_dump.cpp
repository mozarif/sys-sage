#include "sys-sage.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>

using namespace sys_sage;
using json = nlohmann::json;

static void DumpAttributes(const json &obj) { /* TODO */ }

static void CollectComponentsInSubtree(const Component *root, std::unordered_set<const Component *> &components)
{
    components.insert(root);

    for (auto child : root->GetChildren())
        CollectComponentsInSubtree(child, components);
}

int sys_sage::Component::DumpJson(const std::filesystem::path &path) const
{
    json componentSubtree = SubtreeToJson();

    std::unordered_set<const Component *> componentsInSubtree;
    CollectComponentsInSubtree(this, componentsInSubtree);
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
                  relationSubgraph.push_back(relation->ToJson());
            }
        }
    }

    json output = {
        { "componentTree", componentSubtree }
    };
    if (relationSubgraph.size() > 0)
        output["relationGraph"] = relationSubgraph;

    if (path.empty()) {
        std::cout << output.dump(4) << std::endl;
        return 0;
    }

    std::filesystem::path file = path / "topo.json";
    std::ofstream stream ( file );
    if (!stream) {
        std::cerr << "Failed to write to " << file << '\n';
        return 1;
    }
    stream << output.dump(4) << std::endl;

    return 0;
}

json sys_sage::Component::ToJson() const
{
    json obj = {
        { "type", GetComponentTypeStr() },
        { "id", id }
    };

    DumpAttributes(obj);

    return obj;
}

json sys_sage::Relation::ToJson() const
{
    json obj = {
        { "type", GetTypeStr() },
        { "category", category },
        { "id", id },
        { "ordered", ordered }
    };

    if (components.size() > 0) {
        // to get a unique identifier for the component, we take the ComponentTypeStr() and
        // append the ID of the component to it, like so: "<ComponentTypeStr()>-<ComponentID>"

        std::vector<std::string> uniqueIdentifiers (components.size());
        for (size_t i = 0; i < uniqueIdentifiers.size(); i++)
          uniqueIdentifiers[i] = components[i]->GetComponentTypeStr() + ("-" + std::to_string(components[i]->GetId()));

        obj["components"] = uniqueIdentifiers;
    }

    DumpAttributes(obj);

    return obj;
}

json sys_sage::AtomSite::ToJson() const
{
    json obj = QuantumBackend::ToJson();

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

    return obj;
}

json sys_sage::Cache::ToJson() const
{
    json obj = Component::ToJson();

    obj["cacheType"] = cache_type;
    if (cache_size > 0)
        obj["cacheSize"] = cache_size;
    if (cache_associativity_ways > 0)
        obj["cacheAssociativityWays"] = cache_associativity_ways;
    if (cache_line_size > 0)
        obj["cacheLineSize"] = cache_line_size;

    return obj;
}

json sys_sage::Chip::ToJson() const
{
    json obj = Component::ToJson();

    obj["chipType"] = type;
    if (!vendor.empty())
        obj["vendor"] = vendor;
    if (!model.empty())
        obj["model"] = model;

    return obj;
}

json sys_sage::Core::ToJson() const
{
    json obj = Component::ToJson();

#ifdef PROC_CPUINFO
    obj["frequency"] = freq;
#endif

    return obj;
}

json sys_sage::CouplingMap::ToJson() const
{
    json obj = Relation::ToJson();

    obj["fidelity"] = fidelity;

    return obj;
}

json sys_sage::DataPath::ToJson() const
{
    json obj = Relation::ToJson();

    obj["dataPathType"] = dp_type;
    obj["bandwidth"] = bw;
    obj["latency"] = latency;

    return obj;
}

json sys_sage::Memory::ToJson() const
{
    json obj = Component::ToJson();

    obj["volatile"] = is_volatile;
    if (size > 0)
        obj["size"] = size;

    return obj;
}

json sys_sage::Node::ToJson() const
{
    json obj = Component::ToJson();

    return obj;
}

json sys_sage::Numa::ToJson() const
{
    json obj = Component::ToJson();

    if (size > 0)
        obj["size"] = size;

    return obj;
}

json sys_sage::QuantumBackend::ToJson() const
{
    json obj = Component::ToJson();

    obj["numQubits"] = num_qubits;

    return obj;
}

json sys_sage::QuantumGate::ToJson() const
{
    json obj = Relation::ToJson();

    obj["gateSize"] = gate_size;
    obj["name"] = name;
    obj["gateLength"] = gate_length;
    obj["gateType"] = gate_type;
    obj["fidelity"] = fidelity;
    obj["unitary"] = unitary;

    return obj;
}

json sys_sage::Qubit::ToJson() const
{
    json obj = Component::ToJson();

    obj["q1Fidelity"] = q1_fidelity;
    obj["t1"] = t1;
    obj["t2"] = t2;
    obj["readoutFidelity"] = readout_fidelity;
    obj["readoutLength"] = readout_length;
    obj["frequency"] = frequency;
    obj["calibrationTime"] = calibration_time;

    return obj;
}

json sys_sage::Storage::ToJson() const
{
    json obj = Component::ToJson();

    if (size > 0)
        obj["size"] = size;

    return obj;
}

json sys_sage::Subdivision::ToJson() const
{
    json obj = Component::ToJson();

    obj["subdivisionType"] = type;

    return obj;
}

json sys_sage::Thread::ToJson() const
{
    json obj = Component::ToJson();

    return obj;
}

json sys_sage::Topology::ToJson() const
{
    json obj = Component::ToJson();

    return obj;
}

json sys_sage::Component::SubtreeToJson() const
{
    json obj = ToJson();

    if (children.size() > 0) {
        std::vector<json> jsonChildren ( children.size() );

        for (size_t i = 0; i < jsonChildren.size(); i++)
            // this recursive call chain might be very memory heavy
            jsonChildren[i] = children[i]->SubtreeToJson();

      obj["children"] = jsonChildren;
    }

    return obj;
}
