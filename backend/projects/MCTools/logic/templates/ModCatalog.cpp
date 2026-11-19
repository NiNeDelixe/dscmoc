#include "ModCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <mutex>
#include <stdexcept>

#include <drogon/drogon.h>

namespace
{
    constexpr const char* kSeedPath = "data/mods.json";

    const std::vector<std::string>& allowedThemes()
    {
        static const std::vector<std::string> values{"optimization", "visual", "quality-of-life"};
        return values;
    }

    const std::vector<std::string>& allowedLoaders()
    {
        static const std::vector<std::string> values{"fabric", "forge", "neoforge"};
        return values;
    }

    bool isAllowed(const std::vector<std::string>& allowed, const std::string& value)
    {
        return std::find(allowed.begin(), allowed.end(), value) != allowed.end();
    }

    std::once_flag g_initialized;

    std::string trimAscii(std::string value)
    {
        const auto not_space = [](unsigned char c) { return !std::isspace(c); };
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
        value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
        return value;
    }
}  // namespace

std::vector<std::string> ModCatalog::splitCsv(const std::string& csv)
{
    std::vector<std::string> out;
    std::size_t start = 0;
    while (start <= csv.size())
    {
        const auto comma = csv.find(',', start);
        const auto piece = trimAscii(csv.substr(
            start, comma == std::string::npos ? std::string::npos : comma - start));
        if (!piece.empty())
            out.push_back(piece);
        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }
    return out;
}

std::string ModCatalog::joinCsv(const std::vector<std::string>& values)
{
    std::string out;
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
            out.push_back(',');
        out += values[i];
    }
    return out;
}

std::string ModCatalog::escapeSql(const std::string& value)
{
    std::string out;
    out.reserve(value.size() + 2);
    for (const char c : value)
    {
        if (c == '\'')
            out.push_back('\'');
        out.push_back(c);
    }
    return out;
}

ModCatalog::ModItem ModCatalog::toItem(const nlohmann::json& node)
{
    ModItem item;
    item.project_id = node.value("project_id", std::string{});
    item.slug       = node.value("slug", std::string{});
    item.title      = node.value("title", std::string{});
    item.description = node.value("description", std::string{});
    item.icon_url   = node.value("icon_url", std::string{});

    if (node.contains("themes") && node["themes"].is_array())
    {
        for (const auto& t : node["themes"])
            if (t.is_string())
                item.themes.push_back(t.get<std::string>());
    }
    if (node.contains("loaders") && node["loaders"].is_array())
    {
        for (const auto& l : node["loaders"])
            if (l.is_string())
                item.loaders.push_back(l.get<std::string>());
    }
    return item;
}

nlohmann::json ModCatalog::readSeed()
{
    std::ifstream input(kSeedPath);
    if (!input)
    {
        throw std::runtime_error(std::string("Не найден файл каталога модов: ") + kSeedPath);
    }

    try
    {
        return nlohmann::json::parse(input);
    }
    catch (const nlohmann::json::exception& e)
    {
        throw std::runtime_error(std::string("Не удалось разобрать ") + kSeedPath + ": " + e.what());
    }
}

bool ModCatalog::matches(const ModItem& item,
                         const std::vector<std::string>& themes,
                         const std::string& loader)
{
    if (!loader.empty())
    {
        const bool supported =
            std::find(item.loaders.begin(), item.loaders.end(), loader) != item.loaders.end();
        if (!supported)
            return false;
    }

    if (themes.empty())
        return true;

    for (const auto& theme : themes)
    {
        if (std::find(item.themes.begin(), item.themes.end(), theme) != item.themes.end())
            return true;
    }
    return false;
}

std::vector<ModCatalog::ModItem> ModCatalog::listFromSeed(const std::vector<std::string>& themes,
                                                          const std::string& loader)
{
    const nlohmann::json seed = readSeed();

    std::vector<std::pair<int, ModItem>> ordered;
    if (seed.contains("mods") && seed["mods"].is_array())
    {
        for (const auto& node : seed["mods"])
        {
            ModItem item = toItem(node);
            if (item.project_id.empty() || !matches(item, themes, loader))
                continue;
            ordered.emplace_back(node.value("position", 1000), std::move(item));
        }
    }

    std::stable_sort(ordered.begin(), ordered.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });

    std::vector<ModItem> out;
    out.reserve(ordered.size());
    for (auto& [position, item] : ordered)
        out.push_back(std::move(item));
    return out;
}

void ModCatalog::ensureSchemaAndSeed()
{
    auto db = drogon::app().getDbClient();
    if (!db)
    {
        LOG_WARN << "db_clients не настроен: каталог модов будет отдаваться из " << kSeedPath;
        return;
    }

    db->execSqlSync(
        "CREATE TABLE IF NOT EXISTS mods ("
        " project_id TEXT PRIMARY KEY,"
        " slug        TEXT NOT NULL,"
        " title       TEXT NOT NULL,"
        " description TEXT NOT NULL DEFAULT '',"
        " icon_url    TEXT NOT NULL DEFAULT '',"
        " themes      TEXT NOT NULL DEFAULT '',"
        " loaders     TEXT NOT NULL DEFAULT '',"
        " position    INTEGER NOT NULL DEFAULT 1000"
        ");");

    const nlohmann::json seed = readSeed();
    if (!seed.contains("mods") || !seed["mods"].is_array())
        return;

    for (const auto& node : seed["mods"])
    {
        const ModItem item = toItem(node);
        if (item.project_id.empty() || item.title.empty())
        {
            LOG_WARN << "Пропущена запись каталога без project_id или title";
            continue;
        }

        static const std::string sql =
            "INSERT OR REPLACE INTO mods"
            " (project_id, slug, title, description, icon_url, themes, loaders, position)"
            " VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

        db->execSqlSync(sql,
                        item.project_id,
                        item.slug,
                        item.title,
                        item.description,
                        item.icon_url,
                        joinCsv(item.themes),
                        joinCsv(item.loaders),
                        node.value("position", 1000));
    }

    LOG_INFO << "Каталог модов готов: " << seed["mods"].size() << " записей";
}

std::vector<ModCatalog::ModItem> ModCatalog::list(const std::vector<std::string>& themes,
                                                  const std::string& loader)
{
    std::call_once(g_initialized, []() { ensureSchemaAndSeed(); });

    auto db = drogon::app().getDbClient();
    if (!db)
        return listFromSeed(themes, loader);

    // Only allowlisted filter values are interpolated into SQL below.
    std::vector<std::string> safeThemes;
    for (const auto& theme : themes)
    {
        const std::string value = trimAscii(theme);
        if (isAllowed(allowedThemes(), value))
            safeThemes.push_back(value);
        else
            LOG_WARN << "Игнорируется неизвестная тематика фильтра: " << value;
    }

    const std::string safeLoader = isAllowed(allowedLoaders(), loader) ? loader : std::string{};
    if (!safeLoader.empty() != !loader.empty())
        LOG_WARN << "Игнорируется неизвестный загрузчик фильтра: " << loader;

    // Match whole CSV values so visual doesn't match visualization.
    std::string where;
    if (!safeLoader.empty())
        where += "(',' || loaders || ',') LIKE '%," + escapeSql(safeLoader) + ",%'";

    if (!safeThemes.empty())
    {
        std::string themeClause;
        for (const auto& theme : safeThemes)
        {
            if (!themeClause.empty())
                themeClause += " OR ";
            themeClause += "(',' || themes || ',') LIKE '%," + escapeSql(theme) + ",%'";
        }
        if (!where.empty())
            where += " AND ";
        where += "(" + themeClause + ")";
    }

    std::string sql =
        "SELECT project_id, slug, title, description, icon_url, themes, loaders"
        " FROM mods";
    if (!where.empty())
        sql += " WHERE " + where;
    sql += " ORDER BY position, title";

    auto result = db->execSqlSync(sql);

    std::vector<ModItem> items;
    items.reserve(result.size());
    for (const auto& row : result)
    {
        ModItem item;
        item.project_id  = row["project_id"].as<std::string>();
        item.slug        = row["slug"].as<std::string>();
        item.title       = row["title"].as<std::string>();
        item.description = row["description"].as<std::string>();
        item.icon_url    = row["icon_url"].as<std::string>();
        item.themes      = splitCsv(row["themes"].as<std::string>());
        item.loaders     = splitCsv(row["loaders"].as<std::string>());
        items.push_back(std::move(item));
    }
    return items;
}