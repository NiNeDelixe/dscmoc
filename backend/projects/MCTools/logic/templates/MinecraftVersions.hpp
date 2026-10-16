#ifndef TEMPLATES_MINECRAFT_VERIONS_HPP_
#define TEMPLATES_MINECRAFT_VERIONS_HPP_

#include "CoreDefines.hpp"

#include <string>
#include <vector>

#include <cpr/api.h>
#include <nlohmann/json.hpp>
#include <deluxe_lib/core/core.hpp>


class MinecraftVersions
{
    DL_DECLARE_CLASS(MinecraftVersions)

public:
    ~MinecraftVersions() = default;

public:
    struct VersionEntry
    {
        std::string id;
        std::string type;
        std::string url;
        std::string time;
        std::string release_time;
    }; 
    

public:
    std::vector<VersionEntry> getAllVersions() const;


private:
    const std::string m_url = "https://launchermeta.mojang.com/mc/game/version_manifest.json";
};

#endif // TEMPLATES_MINECRAFT_VERIONS_HPP_
