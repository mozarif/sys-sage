#include "sys-sage.hpp"
#include <nlohmann/json.hpp>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <stddef.h>
#include <string>
#include <string_view>

using namespace sys_sage;
using json = nlohmann::json;

static void LoadAttributes(const json &obj) { /* TODO */ }

static int RelationFromJson(Component *root, const json &relationJson)
{
    std::vector<Component *> components;
    for (auto &componentJson : relationJson["components"]) {
        auto uniqueIdentifier = componentJson.get<std::string>();
        const char *start = uniqueIdentifier.data();
        const char *end = uniqueIdentifier.data() + uniqueIdentifier.size();
        size_t pos = uniqueIdentifier.find('-');
        if (pos == std::string::npos)
            return 1;
        std::string_view componentType (start, pos);
        int componentId;
        auto [ptr, ec] = std::from_chars(start + pos + 1, end, componentId);
        if (ptr != end || ec != std::errc())
            return 1;

        ComponentType::type type;
        if (componentType == "GenericComponent")
            type = ComponentType::Generic;
        else if (componentType == "HW_Thread")
            type = ComponentType::Thread;
        else if (componentType == "Core")
            type = ComponentType::Core;
        else if (componentType == "Cache")
            type = ComponentType::Cache;
        else if (componentType == "Subdivision")
            type = ComponentType::Subdivision;
        else if (componentType == "NUMA")
            type = ComponentType::Numa;
        else if (componentType == "Chip")
            type = ComponentType::Chip;
        else if (componentType == "Memory")
            type = ComponentType::Memory;
        else if (componentType == "Storage")
            type = ComponentType::Storage;
        else if (componentType == "Node")
            type = ComponentType::Node;
        else if (componentType == "QuantumBackend")
            type = ComponentType::QuantumBackend;
        else if (componentType == "AtomSite")
            type = ComponentType::AtomSite;
        else if (componentType == "Qubit")
            type = ComponentType::Qubit;
        else if (componentType == "Topology")
            type = ComponentType::Topology;
        else
            return 1;

        auto component = root->GetDescendantById(componentId, type);
        if (component == nullptr)
            return 1;

        components.push_back(component);
    }

    std::string type = relationJson["type"].get<std::string>();
    if (type == "Relation") {
        new Relation(components, relationJson["id"].get<int>(),
                     relationJson["ordered"].get<bool>(),
                     relationJson["category"].get<RelationCategory::type>());
    } else if (type == "DataPath") {
        auto dataPath = new DataPath(components[0], components[1],
                                     relationJson["ordered"].get<bool>() ? DataPathOrientation::Oriented : DataPathOrientation::Bidirectional,
                                     relationJson["dataPathType"].get<DataPathType::type>());
        dataPath->SetBandwidth(relationJson["bandwidth"].get<double>());
        dataPath->SetLatency(relationJson["latency"].get<double>());
    } else if (type == "QuantumGate") {
        new QuantumGate(components, relationJson["id"].get<int>(),
                        relationJson["ordered"].get<bool>(),
                        relationJson["gateSize"].get<size_t>(),
                        relationJson["name"].get<std::string>(),
                        relationJson["gateLength"].get<int>(),
                        relationJson["gateType"].get<QuantumGateType::type>(),
                        relationJson["fidelity"].get<double>(),
                        relationJson["unitary"].get<std::string>());
    } else if (type == "CouplingMap") {
        auto couplingMap = new CouplingMap(components,
                                           relationJson["id"].get<int>(),
                                           relationJson["ordered"].get<bool>());
        couplingMap->SetFidelity(relationJson["fidelity"].get<double>());
    } else {
        return 1;
    }

    LoadAttributes(relationJson);

    return 0;
}

static AtomSite *AtomSiteFromJson(const json &obj)
{
    auto atomSite = new AtomSite;
    atomSite->properties = {
        .nRows = obj["nRows"].get<int>(),
        .nColumns = obj["nColumns"].get<int>(),
        .nAods = obj["nAods"].get<int>(),
        .nAodIntermediateLevels = obj["nAodIntermediateLevels"].get<int>(),
        .nAodCoordinates = obj["nAodCoordinates"].get<int>(),
        .interQubitDistance = obj["interQubitDistance"].get<double>(),
        .interactionRadius = obj["interactionRadius"].get<double>(),
        .blockingFactor = obj["blockingFactor"].get<double>()
    };
    atomSite->SetNumQubits(obj["numQubits"].get<int>());

    atomSite->SetId(obj["id"].get<int>());
    
    return atomSite;
}

static Cache *CacheFromJson(const json &obj)
{
    long long cacheSize = -1;
    if (auto it = obj.find("cacheSize"); it != obj.end())
        cacheSize = it->get<long long>();

    int cacheAssociativityWays = -1;
    if (auto it = obj.find("cacheAssociativityWays"); it != obj.end())
        cacheAssociativityWays = it->get<int>();

    int cacheLineSize = -1;
    if (auto it = obj.find("cacheLineSize"); it != obj.end())
        cacheLineSize = it->get<int>();

    auto cache = new Cache(obj["id"].get<int>(),
                           std::stoi(obj["cacheType"].get<std::string>()), cacheSize,
                           cacheAssociativityWays, cacheLineSize);

    return cache;
}

static Chip *ChipFromJson(const json &obj)
{
    std::string vendor ("");
    if (auto it = obj.find("vendor"); it != obj.end())
        vendor = it->get<std::string>();

    std::string model ("");
    if (auto it = obj.find("model"); it != obj.end())
        model = it->get<std::string>();

    auto chip = new Chip(obj["id"].get<int>(), "Chip",
                         obj["chipType"].get<ChipType::type>(),
                         vendor, model);

    return chip;
}

static Core *CoreFromJson(const json &obj)
{
    auto core = new Core(obj["id"].get<int>());

#ifdef PROC_CPUINFO
    if (auto it = obj.find("frequency"); it != obj.end())
      core->SetFreq(it->get<double>());
#endif

    return core;
}

static Memory *MemoryFromJson(const json &obj)
{
    long long size = -1;
    if (auto it = obj.find("size"); it != obj.end())
        size = it->get<long long>();

    auto memory = new Memory(size, obj["volatile"].get<bool>());
    memory->SetId(obj["id"].get<int>());

    return memory;
}

static Node *NodeFromJson(const json &obj)
{
    auto node = new Node(obj["id"].get<int>());

    return node;
}

static Numa *NumaFromJson(const json &obj)
{
    long long size = -1;
    if (auto it = obj.find("size"); it != obj.end())
        size = it->get<long long>();

    auto node = new Numa(obj["id"].get<int>(), size);

    return node;
}

static QuantumBackend *QuantumBackendFromJson(const json &obj)
{
    auto quantumBackend = new QuantumBackend(obj["id"].get<int>());
    quantumBackend->SetNumQubits(obj["numQubits"].get<int>());

    return quantumBackend;
}

static Qubit *QubitFromJson(const json &obj)
{
    auto qubit = new Qubit(obj["id"].get<int>());
    qubit->SetProperties(obj["t1"].get<double>(), obj["t2"].get<double>(),
                         obj["readoutFidelity"].get<double>(),
                         obj["q1Fidelity"].get<double>(),
                         obj["readoutLength"].get<double>());

    return qubit;
}

static Storage *StorageFromJson(const json &obj)
{
    long long size = -1;
    if (auto it = obj.find("size"); it != obj.end())
        size = it->get<long long>();
    auto storage = new Storage(size);
    
    storage->SetId(obj["id"].get<int>());

    return storage;
}

static Subdivision *SubdivisionFromJson(const json &obj)
{
    auto subdivision = new Subdivision(obj["id"].get<int>());
    subdivision->SetSubdivisionType(obj["subdivisionType"].get<SubdivisionType::type>());

    return subdivision;
}

static Thread *ThreadFromJson(const json &obj)
{
    auto thread = new Thread(obj["id"].get<int>());

    return thread;
}

static Topology *TopologyFromJson(const json &obj)
{
    auto topology = new Topology;
    topology->SetId(obj["id"].get<int>());

    return topology;
}

static Component *ComponentFromJson(const json &componentJson)
{
    Component *component;

    std::string componentType = componentJson["type"].get<std::string>();
    if (componentType == "GenericComponent")
        component = new Component(componentJson["id"].get<int>(), "Component");
    else if (componentType == "HW_Thread")
        component = ThreadFromJson(componentJson);
    else if (componentType == "Core")
        component = CoreFromJson(componentJson);
    else if (componentType == "Cache")
        component = CacheFromJson(componentJson);
    else if (componentType == "Subdivision")
        component = SubdivisionFromJson(componentJson);
    else if (componentType == "NUMA")
        component = NumaFromJson(componentJson);
    else if (componentType == "Chip")
        component = ChipFromJson(componentJson);
    else if (componentType == "Memory")
        component = MemoryFromJson(componentJson);
    else if (componentType == "Storage")
        component = StorageFromJson(componentJson);
    else if (componentType == "Node")
        component = NodeFromJson(componentJson);
    else if (componentType == "QuantumBackend")
        component = QuantumBackendFromJson(componentJson);
    else if (componentType == "AtomSite")
        component = AtomSiteFromJson(componentJson);
    else if (componentType == "Qubit")
        component = QubitFromJson(componentJson);
    else if (componentType == "Topology")
        component = TopologyFromJson(componentJson);
    else
        return nullptr;

    LoadAttributes(componentJson);

    if (auto it = componentJson.find("children"); it != componentJson.end()) {
        for (auto &childJson : *it) {
            Component *child = ComponentFromJson(childJson);
            if (child == nullptr) {
                delete component;
                return nullptr;
            }

            child->SetParent(component);
            component->InsertChild(child);
        }
    }

    return component;
}

Component *sys_sage::Component::LoadJson(const std::filesystem::path &path)
{
    std::ifstream stream ( path );
    if (!stream)
        return nullptr;

    const json input = json::parse(stream);
    stream.close();

    Component *root = ComponentFromJson(input["componentTree"]);

    if (auto it = input.find("relationGraph"); it != input.end()) {
        for (auto &relation : *it) {
            if (RelationFromJson(root, relation) != 0) {
                delete root;
                return nullptr;
            }
        }
    }

    return root;
}
