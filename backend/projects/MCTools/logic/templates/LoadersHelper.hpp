#ifndef TEMPLATES_LOADERS_HELPER_HPP_
#define TEMPLATES_LOADERS_HELPER_HPP_

#include "CoreDefines.hpp"

#include <string>
#include <vector>

#include <cpr/api.h>
#include <nlohmann/json.hpp>
#include <deluxe_lib/core/core.hpp>

class LoadersHelper
{
    DL_DECLARE_CLASS(LoadersHelper)

public:
    ~LoadersHelper() = default;

public:
    struct LoaderVersionEntry
    {
        std::string loader;
        std::string version;
        bool is_stable;
        std::string mc_version;
    };

    enum Loaders
    {
        ALL,
        FABRIC,
        NEOFORGE,
        FORGE
    };

public:
    std::vector<LoaderVersionEntry> getAllLoaderVersions(const std::string& mc_version, const Loaders& loader = ALL) const;

    std::vector<LoaderVersionEntry> getAllFabricVersions(const std::string& mc_version) const;
    std::vector<LoaderVersionEntry> getAllNeoForgeVersions(const std::string& mc_version) const;
    std::vector<LoaderVersionEntry> getAllForgeVersions(const std::string& mc_version) const;

private:
    const std::string m_url_fabric = "https://meta.fabricmc.net/v2/versions/loader";
    const std::string m_url_neoforge = "https://maven.neoforged.net/releases/net/neoforged/neoforge/maven-metadata.xml";
    const std::string m_url_forge = "https://maven.minecraftforge.net/net/minecraftforge/forge/maven-metadata.xml";
};

#endif // TEMPLATES_LOADERS_HELPER_HPP_
