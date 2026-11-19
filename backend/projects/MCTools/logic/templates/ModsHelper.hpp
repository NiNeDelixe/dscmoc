#ifndef TEMPLATES_MODS_HELPER_HPP_
#define TEMPLATES_MODS_HELPER_HPP_

#include "CoreDefines.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <cpr/api.h>
#include <nlohmann/json.hpp>
#include <deluxe_lib/core/core.hpp>

class ModsHelper
{
    DL_DECLARE_CLASS(ModsHelper)

public:
    ~ModsHelper() = default;

public:
    struct ModEntry
    {
        std::string id;       ///< project_id Modrinth
        std::string slug;
        std::string title;
        std::string description;
        std::string author;
        std::string icon_url;
        std::string project_url;
        std::int64_t downloads = 0;
    };

    struct ModFileEntry
    {
        std::string mod_id;
        std::string title;
        std::string filename;   // No mods/ prefix.
        std::string url;
        std::string sha1;
        std::string sha512;
        std::int64_t size = 0;
        std::string version_id;
        std::string loader;
        std::string mc_version;
    };

    struct RequestError
    {
        int status_code = 0;
        std::string message;
    };

public:
    // An empty mc_version disables version filtering.
    std::vector<ModEntry> search(const std::string& query,
                                 const std::string& loader,
                                 const std::string& mc_version,
                                 std::size_t limit = 30,
                                 std::size_t offset = 0) const;

    nlohmann::json getProjectVersions(const std::string& mod_id,
                                      const std::string& loader,
                                      const std::string& mc_version) const;

    // Returns false if no compatible file is available.
    bool resolveBestFile(const ModEntry& mod,
                         const std::string& loader,
                         const std::string& mc_version,
                         ModFileEntry& out) const;

    static bool isKnownLoader(const std::string& loader);

private:
    nlohmann::json getJson(const std::string& url) const;

    static std::string urlEncodeComponent(const std::string& value);

    static std::string sanitizeFilename(const std::string& name);

private:
    const std::string m_base_url = "https://api.modrinth.com/v2";
    const std::string m_cdn_url  = "https://cdn.modrinth.com";
    const std::string m_user_agent = "dscmoc-mctools/1.0 (MCTools modpack wizard)";
};

#endif // TEMPLATES_MODS_HELPER_HPP_