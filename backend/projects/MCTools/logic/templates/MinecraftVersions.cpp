#include "MinecraftVersions.hpp"

std::vector<MinecraftVersions::VersionEntry> MinecraftVersions::fetchAllVersions() const
{
    cpr::Response response = cpr::Get(cpr::Url{m_url});

    if (response.status_code != 200)
    {
        if (!response.error.message.empty())
        {
            throw std::runtime_error(response.error.message);
        }
        throw std::runtime_error("error in fetching versions");
    }

    nlohmann::json json = nlohmann::json::parse(response.text);
    nlohmann::json versions = json["versions"];

    std::vector<VersionEntry> result;
    result.reserve(versions.size());

    for (size_t i = 0; i < versions.size(); i++)
    {
        const std::string type = versions[i]["type"].template get<std::string>();
        if (type != "release")
            continue;

        VersionEntry entry;
        entry.id = versions[i]["id"].template get<std::string>();
        entry.type = type;
        entry.url = versions[i]["url"].template get<std::string>();
        entry.time = versions[i]["time"].template get<std::string>();
        entry.release_time = versions[i]["releaseTime"].template get<std::string>();
        result.push_back(std::move(entry));
    }

    return result;
}

std::vector<MinecraftVersions::VersionEntry> MinecraftVersions::getAllReleases() const
{
    const auto now = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_cache_valid && now - m_cached_at < m_cache_ttl)
    {
        return m_cache;
    }

    // Keep the lock during the fetch so concurrent requests share one refresh.
    std::vector<VersionEntry> fresh = fetchAllVersions();

    m_cache = std::move(fresh);
    m_cached_at = now;
    m_cache_valid = true;
    return m_cache;
}

void MinecraftVersions::invalidateCache() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache_valid = false;
    m_cache.clear();
}