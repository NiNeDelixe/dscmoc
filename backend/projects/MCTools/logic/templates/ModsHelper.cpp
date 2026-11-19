#include "ModsHelper.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace
{
    constexpr std::size_t kMaxSearchLimit = 100;
}

bool ModsHelper::isKnownLoader(const std::string& loader)
{
    return loader == "fabric" || loader == "forge" || loader == "neoforge";
}

std::string ModsHelper::urlEncodeComponent(const std::string& value)
{
    // RFC 3986 encoding for query values, including JSON facets.
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::string out;
    out.reserve(value.size() * 3);

    for (const unsigned char c : value)
    {
        const bool unreserved = std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
        if (unreserved)
        {
            out.push_back(static_cast<char>(c));
        }
        else
        {
            out.push_back('%');
            out.push_back(kHex[c >> 4]);
            out.push_back(kHex[c & 0x0F]);
        }
    }
    return out;
}

std::string ModsHelper::sanitizeFilename(const std::string& name)
{
    std::string out;
    out.reserve(name.size());
    for (const char c : name)
    {
        out.push_back((std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_')
                          ? c
                          : '_');
    }
    if (out.empty())
        out = "mod";
    return out;
}

nlohmann::json ModsHelper::getJson(const std::string& url) const
{
    cpr::Response response = cpr::Get(
        cpr::Url{url},
        cpr::Header{{"User-Agent", m_user_agent}, {"Accept", "application/json"}});

    if (response.status_code != 200)
    {
        if (!response.error.message.empty())
            throw std::runtime_error("Modrinth: " + response.error.message);
        throw std::runtime_error("Modrinth вернул HTTP " + std::to_string(response.status_code));
    }

    try
    {
        return nlohmann::json::parse(response.text);
    }
    catch (const nlohmann::json::exception& e)
    {
        throw std::runtime_error(std::string("Modrinth: не удалось разобрать ответ: ") + e.what());
    }
}

std::vector<ModsHelper::ModEntry> ModsHelper::search(const std::string& query,
                                                     const std::string& loader,
                                                     const std::string& mc_version,
                                                     std::size_t limit,
                                                     std::size_t offset) const
{
    if (!isKnownLoader(loader))
        throw std::runtime_error("Неизвестный загрузчик модов: " + loader);

    // Modrinth ANDs the facet groups.
    nlohmann::json facets = nlohmann::json::array();
    facets.push_back(nlohmann::json::array({"project_type:mod"}));
    facets.push_back(nlohmann::json::array({"categories:" + loader}));
    if (!mc_version.empty())
        facets.push_back(nlohmann::json::array({"versions:" + mc_version}));

    // windows.h defines min and max as macros.
    std::size_t clamped = limit < 1 ? 1 : limit;
    if (clamped > kMaxSearchLimit)
        clamped = kMaxSearchLimit;

    std::string url = m_base_url + "/search?query=" + urlEncodeComponent(query);
    url += "&facets=" + urlEncodeComponent(facets.dump());
    url += "&limit=" + std::to_string(clamped);
    // Modrinth's index parameter chooses the sort order, not the offset.
    url += "&offset=" + std::to_string(offset);

    const nlohmann::json body = getJson(url);
    const nlohmann::json& hits = body["hits"];
    if (!hits.is_array())
        throw std::runtime_error("Modrinth: в ответе поиска нет массива hits");

    std::vector<ModEntry> result;
    result.reserve(hits.size());

    for (const auto& hit : hits)
    {
        ModEntry entry;
        entry.id          = hit.value("project_id", std::string{});
        entry.slug        = hit.value("slug", std::string{});
        entry.title       = hit.value("title", std::string{});
        entry.description = hit.value("description", std::string{});
        entry.author      = hit.value("author", std::string{});
        entry.icon_url    = hit.value("icon_url", std::string{});
        entry.downloads   = hit.value("downloads", std::int64_t{0});
        entry.project_url = "https://modrinth.com/mod/" + (entry.slug.empty() ? entry.id : entry.slug);

        if (entry.id.empty())
            continue;
        result.push_back(std::move(entry));
    }

    return result;
}

nlohmann::json ModsHelper::getProjectVersions(const std::string& mod_id,
                                              const std::string& loader,
                                              const std::string& mc_version) const
{
    if (!isKnownLoader(loader))
        throw std::runtime_error("Неизвестный загрузчик модов: " + loader);

    const std::string loaders_param = nlohmann::json::array({loader}).dump();
    std::string game_versions_param = nlohmann::json::array({mc_version}).dump();

    std::string url = m_base_url + "/project/" + urlEncodeComponent(mod_id) + "/version";
    url += "?loaders=" + urlEncodeComponent(loaders_param);
    if (!mc_version.empty())
        url += "&game_versions=" + urlEncodeComponent(game_versions_param);

    const nlohmann::json body = getJson(url);
    if (!body.is_array())
        throw std::runtime_error("Modrinth: ответ со списком версий не является массивом");

    return body;
}

bool ModsHelper::resolveBestFile(const ModEntry& mod,
                                 const std::string& loader,
                                 const std::string& mc_version,
                                 ModFileEntry& out) const
{
    nlohmann::json versions;
    try
    {
        versions = getProjectVersions(mod.id, loader, mc_version);
    }
    catch (const std::exception& e)
    {
        const std::string label = mod.title.empty() ? mod.id : mod.title + " (" + mod.id + ")";
        throw std::runtime_error("не удалось получить версии " + loader + " для " + label +
                                 ": " + e.what());
    }

    // Modrinth returns the newest versions first.
    for (const auto& version : versions)
    {
        const nlohmann::json& files = version["files"];
        if (!files.is_array())
            continue;

        const nlohmann::json* chosen = nullptr;
        for (const auto& file : files)
        {
            chosen = &file;
            if (file.value("primary", false))
                break;
        }

        if (chosen == nullptr || !chosen->is_object())
            continue;

        const std::string url = chosen->value("url", std::string{});
        if (url.empty())
            continue;

        // A Modrinth release may also include resource packs or other ZIP files.
        const std::string filename = chosen->value("filename", std::string{});
        if (!filename.empty() &&
            filename.size() > 4 &&
            filename.compare(filename.size() - 4, 4, ".jar") != 0)
        {
            continue;
        }

        out.mod_id      = mod.id;
        out.title       = mod.title.empty() ? mod.slug : mod.title;
        out.filename    = filename.empty() ? (sanitizeFilename(mod.slug) + ".jar")
                                           : sanitizeFilename(filename);
        out.url         = url;
        out.sha1        = (*chosen)["hashes"].value("sha1", std::string{});
        out.sha512      = (*chosen)["hashes"].value("sha512", std::string{});
        out.size        = (*chosen).value("size", std::int64_t{0});
        out.version_id  = version.value("id", std::string{});
        out.loader      = loader;
        out.mc_version  = mc_version;
        return true;
    }

    return false;
}