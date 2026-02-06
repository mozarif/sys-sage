#include "sys-sage.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <string>

using namespace sys_sage;
using json = nlohmann::json;

static void LoadAttributes(const Component *parent, const json &attribute) { /* TODO */ }

static Component *ComponentFromJson(Component *parent, const json &componentJson)
{
    auto component = new Component(parent, componentJson["id"].get<int>(), "GenericComponent");
    
    return component;
}

static AtomSite *AtomSiteFromJson(Component *parent, const json &atomSiteJson)
{
    auto atomSite = new AtomSite;
    atomSite->properties = {
        .nRows = atomSiteJson["nRows"].get<int>(),
        .nColumns = atomSiteJson["nColumns"].get<int>(),
        .nAods = atomSiteJson["nAods"].get<int>(),
        .nAodIntermediateLevels = atomSiteJson["nAodIntermediateLevels"].get<int>(),
        .nAodCoordinates = atomSiteJson["nAodCoordinates"].get<int>(),
        .interQubitDistance = atomSiteJson["interQubitDistance"].get<double>(),
        .interactionRadius = atomSiteJson["interactionRadius"].get<double>(),
        .blockingFactor = atomSiteJson["blockingFactor"].get<double>()
    };
    if (parent) {
        atomSite->SetParent(parent);
        parent->InsertChild(atomSite);
    }
    
    return atomSite;
}

static Cache *CacheFromJson(Component *parent, const json &cacheJson)
{
    long long cacheSize = -1;
    if (auto it = cacheJson.find("cacheSize"); it != cacheJson.end())
        cacheSize = it->get<long long>();

    int cacheAssociativityWays = -1;
    if (auto it = cacheJson.find("cacheAssociativityWays"); it != cacheJson.end())
        cacheAssociativityWays = it->get<int>();

    int cacheLineSize = -1;
    if (auto it = cacheJson.find("cacheLineSize"); it != cacheJson.end())
        cacheLineSize = it->get<int>();

    auto cache = new Cache(parent, cacheJson["id"].get<int>(),
                           cacheJson["cacheType"].get<std::string>(), cacheSize,
                           cacheAssociativityWays, cacheLineSize);

    return cache;
}

static Chip *ChipFromJson(Component *parent, const json &chipJson)
{
    std::string vendor ("");
    if (auto it = chipJson.find("vendor"); it != chipJson.end())
        vendor = it->get<std::string>();

    std::string model ("");
    if (auto it = chipJson.find("model"); it != chipJson.end())
        model = it->get<std::string>();

    auto chip = new Chip(parent, chipJson["id"].get<int>(), "Chip",
                         chipJson["chipType"].get<ChipType::type>(),
                         vendor, model);

    return chip;
}

Component *sys_sage::Component::LoadJson(const std::filesystem::path &path)
{
    std::ifstream stream ( path );
    if (!stream)
        return nullptr;

    const json input = json::parse(stream);
    stream.close();

    return nullptr;
}
