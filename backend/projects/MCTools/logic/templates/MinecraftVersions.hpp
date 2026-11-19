#ifndef TEMPLATES_MINECRAFT_VERIONS_HPP_
#define TEMPLATES_MINECRAFT_VERIONS_HPP_

#include "CoreDefines.hpp"

#include <string>
#include <vector>
#include <chrono>
#include <mutex>

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
    std::vector<VersionEntry> getAllReleases() const;

    void invalidateCache() const;

private:
    std::vector<VersionEntry> fetchAllVersions() const;

private:
    const std::string m_url = "https://launchermeta.mojang.com/mc/game/version_manifest.json";

    mutable std::mutex m_mutex;
    mutable std::vector<VersionEntry> m_cache;
    mutable std::chrono::steady_clock::time_point m_cached_at{};
    mutable bool m_cache_valid = false;

    static constexpr std::chrono::seconds m_cache_ttl{std::chrono::minutes(30)};
};

#endif // TEMPLATES_MINECRAFT_VERIONS_HPP_
