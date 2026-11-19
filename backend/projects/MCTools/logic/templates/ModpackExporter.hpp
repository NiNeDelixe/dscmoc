#ifndef TEMPLATES_MODPACK_EXPORTER_HPP_
#define TEMPLATES_MODPACK_EXPORTER_HPP_

#include "CoreDefines.hpp"
#include "projects/MCTools/logic/templates/ModsHelper.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>
#include <deluxe_lib/core/core.hpp>

class ModpackExporter
{
public:
    ModpackExporter() = default;
    ~ModpackExporter() = default;

public:
    static const std::string kTempRootName;

    enum class Format
    {
        ModsZip,
        Mrpack,
        PakkuProject
    };

    struct Request
    {
        std::string loader;
        std::string mc_version;
        std::string loader_version; // Empty means auto-select.
        std::vector<ModsHelper::ModEntry> mods;
        Format format = Format::ModsZip;
        std::string pack_name = "MCTools";
        std::string java_args;
        std::string instructions;
    };

    struct Result
    {
        std::string archive_path;
        std::string download_name;
        std::vector<std::string> warnings; // Mods with no compatible file.
    };

    // Drogon reads the file while sending it; later exports clean up old archives.
    Result build(const Request& request) const;
    nlohmann::json prepareBrowserExport(const Request& request) const;

    static bool parseFormat(const std::string& value, Format& out);

private:
    struct Plan
    {
        std::vector<ModsHelper::ModFileEntry> files;
        std::vector<std::pair<std::string, std::string>> documents;
        std::string download_name;
        std::vector<std::string> warnings;
    };

    Plan prepare(const Request& request) const;

    void downloadMod(const ModsHelper::ModFileEntry& entry,
                     const std::filesystem::path& mods_dir) const;

    nlohmann::json buildMrpackIndex(const Request& request,
                                    const std::vector<ModsHelper::ModFileEntry>& files) const;

    std::string buildPakkuJson(const Request& request,
                               const std::vector<ModsHelper::ModFileEntry>& files) const;

    std::string buildReadme(const Request& request,
                            const std::vector<ModsHelper::ModFileEntry>& files) const;

private:
    static ModsHelper& mods();
};

#endif // TEMPLATES_MODPACK_EXPORTER_HPP_
