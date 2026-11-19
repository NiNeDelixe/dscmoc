#ifndef TEMPLATES_MOD_CATALOG_HPP_
#define TEMPLATES_MOD_CATALOG_HPP_

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class ModCatalog
{
public:
    struct ModItem
    {
        std::string project_id;
        std::string slug;
        std::string title;
        std::string description;
        std::string icon_url;
        std::vector<std::string> themes;
        std::vector<std::string> loaders;
    };

    ModCatalog() = delete;

    // Any matching theme is enough; empty filters are ignored.
    static std::vector<ModItem> list(const std::vector<std::string>& themes,
                                     const std::string& loader);

    static nlohmann::json readSeed();

private:
    static void ensureSchemaAndSeed();

    static std::vector<ModItem> listFromSeed(const std::vector<std::string>& themes,
                                             const std::string& loader);

    static ModItem toItem(const nlohmann::json& node);

    static std::vector<std::string> splitCsv(const std::string& csv);

    static std::string joinCsv(const std::vector<std::string>& values);

    static std::string escapeSql(const std::string& value);

    static bool matches(const ModItem& item,
                        const std::vector<std::string>& themes,
                        const std::string& loader);
};

#endif // TEMPLATES_MOD_CATALOG_HPP_