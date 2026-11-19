#include "ModpackExporter.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>
#include <system_error>

#include <minizip/zip.h>

#ifdef _WIN32
#    include <minizip/iowin32.h>
#endif

const std::string ModpackExporter::kTempRootName = ".mctools_exports";

ModsHelper& ModpackExporter::mods()
{
    return ModsHelper::getInstance();
}

namespace
{
    class ZipWriter
    {
    public:
        explicit ZipWriter(const std::filesystem::path& path)
        {
#ifdef _WIN32
            // Use UTF-16 for paths outside the Windows code page.
            zlib_filefunc64_def file_funcs{};
            fill_win32_filefunc64W(&file_funcs);
            const std::wstring wide = path.wstring();
            m_zip = zipOpen2_64(wide.c_str(), APPEND_STATUS_CREATE, nullptr, &file_funcs);
#else
            const std::string narrow = path.string();
            m_zip = zipOpen2_64(narrow.c_str(), APPEND_STATUS_CREATE, nullptr, nullptr);
#endif
            if (m_zip == nullptr)
                throw std::runtime_error("Не удалось создать архив: " + path.string());
            m_path = path;
        }

        ~ZipWriter()
        {
            try
            {
                close();
            }
            catch (...)
            {
                // The explicit close() reports errors on the normal path.
            }
        }

        ZipWriter(const ZipWriter&) = delete;
        ZipWriter& operator=(const ZipWriter&) = delete;

        void addBuffer(const std::string& entry_name, const std::string& data)
        {
            openEntry(entry_name);
            if (!data.empty())
                write(data.data(), data.size());
            closeEntry();
        }

        void addFileFromDisk(const std::string& entry_name, const std::filesystem::path& source)
        {
            std::ifstream input(source, std::ios::binary);
            if (!input)
                throw std::runtime_error("Не удалось прочитать скачанный файл: " + source.string());

            openEntry(entry_name);
            std::vector<char> chunk(64 * 1024);
            while (input)
            {
                input.read(chunk.data(), static_cast<std::streamsize>(chunk.size()));
                const auto got = input.gcount();
                if (got <= 0)
                    break;
                write(chunk.data(), static_cast<std::size_t>(got));
            }
            closeEntry();
        }

        void close(const std::string& comment = {})
        {
            if (m_zip == nullptr)
                return;
            if (m_entry_open)
                closeEntry();

            const int rc = zipClose(m_zip, comment.empty() ? nullptr : comment.c_str());
            m_zip = nullptr;
            if (rc != ZIP_OK)
                throw std::runtime_error("Ошибка при закрытии архива " + m_path.string());
        }

    private:
        static void check(int rc, const char* what)
        {
            if (rc != ZIP_OK)
                throw std::runtime_error(std::string("Ошибка архива (") + what + "), код " +
                                         std::to_string(rc));
        }

        void openEntry(const std::string& entry_name)
        {
            if (zipAlreadyThere(m_zip, entry_name.c_str()) != 0)
                throw std::runtime_error("Дублирующаяся запись в архиве: " + entry_name);

            const int rc = zipOpenNewFileInZip3_64(
                m_zip,
                entry_name.c_str(),
                nullptr,
                nullptr,
                0,
                nullptr,
                0,
                nullptr,
                Z_DEFLATED,
                Z_DEFAULT_COMPRESSION,
                0,          // raw
                -MAX_WBITS,  // windowBits: raw DEFLATE
                DEF_MEM_LEVEL,
                Z_DEFAULT_STRATEGY,
                nullptr,  // password
                0,        // crcForCrypting
                1);       // Zip64 for large packs
            check(rc, "zipOpenNewFileInZip3_64");
            m_entry_open = true;
        }

        void write(const char* data, std::size_t size)
        {
            check(zipWriteInFileInZip(m_zip, data, static_cast<unsigned>(size)),
                  "zipWriteInFileInZip");
        }

        void closeEntry()
        {
            check(zipCloseFileInZip(m_zip), "zipCloseFileInZip");
            m_entry_open = false;
        }

    private:
        zipFile m_zip = nullptr;
        bool m_entry_open = false;
        std::filesystem::path m_path;
    };

    std::string toLowerAscii(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    std::string trimAscii(std::string value)
    {
        const auto not_space = [](unsigned char c) { return !std::isspace(c); };
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
        value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
        return value;
    }

    std::string escapeToml(const std::string& value)
    {
        std::string out;
        out.reserve(value.size());
        for (const char c : value)
        {
            switch (c)
            {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:   out.push_back(c);
            }
        }
        return out;
    }

    std::string safeDownloadName(const std::string& raw, const char* extension)
    {
        std::string base = trimAscii(raw);
        if (base.empty())
            base = "mctools-modpack";

        std::string out;
        out.reserve(base.size() + 16);
        for (const unsigned char c : base)
        {
            out.push_back((std::isalnum(c) || c == '-' || c == '_' || c == '.') ? static_cast<char>(c)
                                                                                 : '_');
        }
        while (!out.empty() && out.front() == '.')
            out.erase(out.begin());

        if (out.empty())
            out = "mctools-modpack";

        return out + extension;
    }

    std::filesystem::path makeTempDir()
    {
        std::error_code ec;
        const auto root = std::filesystem::absolute(ModpackExporter::kTempRootName, ec);
        if (ec)
            throw std::runtime_error("Не удалось определить рабочий каталог: " + ec.message());

        std::filesystem::create_directories(root, ec);
        if (ec)
            throw std::runtime_error("Не удалось создать " + root.string() + ": " + ec.message());

        const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        constexpr std::int64_t kMaxAgeSeconds = 3600;

        for (const auto& entry : std::filesystem::directory_iterator(root, ec))
        {
            const std::string name = entry.path().filename().string();
            const auto dash = name.find('-');
            if (dash == std::string::npos)
                continue;

            try
            {
                const auto stamp = std::stoll(name.substr(0, dash));
                if (now - stamp > kMaxAgeSeconds)
                    std::filesystem::remove_all(entry.path(), ec);
            }
            catch (const std::exception&)
            {
                // Leave entries without a valid timestamp alone.
            }
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        const auto dir = root / (std::to_string(now) + "-" + std::to_string(gen()));

        std::filesystem::create_directories(dir, ec);
        if (ec)
            throw std::runtime_error("Не удалось создать временный каталог: " + ec.message());

        return dir;
    }
}  // namespace

bool ModpackExporter::parseFormat(const std::string& value, Format& out)
{
    const std::string normalized = toLowerAscii(trimAscii(value));
    if (normalized == "mods-zip")
    {
        out = Format::ModsZip;
        return true;
    }
    if (normalized == "mrpack")
    {
        out = Format::Mrpack;
        return true;
    }
    if (normalized == "pakku-project")
    {
        out = Format::PakkuProject;
        return true;
    }
    return false;
}

ModpackExporter::Plan ModpackExporter::prepare(const Request& request) const
{
    if (!ModsHelper::isKnownLoader(request.loader))
        throw std::runtime_error("Неизвестный загрузчик модов: " + request.loader);
    if (trimAscii(request.mc_version).empty())
        throw std::runtime_error("Не выбрана версия Minecraft");
    if (request.mods.empty())
        throw std::runtime_error("Не выбрано ни одного мода");

    Plan plan;

    auto& files = plan.files;
    files.reserve(request.mods.size());
    for (const auto& mod : request.mods)
    {
        ModsHelper::ModFileEntry entry;
        if (mods().resolveBestFile(mod, request.loader, request.mc_version, entry))
        {
            files.push_back(std::move(entry));
        }
        else
        {
            plan.warnings.push_back(mod.title.empty() ? mod.id : mod.title);
        }
    }

    if (files.empty())
        throw std::runtime_error("Ни один из выбранных модов не поддерживает " + request.loader +
                                 " " + request.mc_version);

    if (request.format == Format::Mrpack)
    {
        plan.documents.emplace_back("modrinth.index.json", buildMrpackIndex(request, files).dump(2));
        plan.download_name = safeDownloadName(request.pack_name, ".mrpack");
        plan.files.clear(); // The launcher downloads the mods.
    }
    else
    {
        if (request.format == Format::PakkuProject)
            plan.documents.emplace_back("pakku.json", buildPakkuJson(request, files));
        plan.documents.emplace_back("README.txt", buildReadme(request, files));
        plan.download_name = safeDownloadName(request.pack_name,
            request.format == Format::PakkuProject ? "-pakku-project.zip" : "-mods.zip");
    }
    return plan;
}

nlohmann::json ModpackExporter::prepareBrowserExport(const Request& request) const
{
    const auto plan = prepare(request);
    auto files = nlohmann::json::array();
    for (const auto& entry : plan.files)
        files.push_back({{"path", "mods/" + entry.filename}, {"url", entry.url},
                         {"size", entry.size}, {"sha512", entry.sha512}});
    return {{"filename", plan.download_name}, {"warnings", plan.warnings},
            {"documents", plan.documents}, {"files", std::move(files)}};
}

ModpackExporter::Result ModpackExporter::build(const Request& request) const
{
    auto plan = prepare(request);
    const auto temp_dir = makeTempDir();
    const auto archive = temp_dir / (request.format == Format::Mrpack ? "pack.mrpack" : "pack.zip");

    try
    {
        const auto mods_dir = temp_dir / "mods";
        if (!plan.files.empty())
        {
            std::filesystem::create_directories(mods_dir);
            for (const auto& entry : plan.files)
                downloadMod(entry, mods_dir);
        }
        ZipWriter zip(archive);
        for (const auto& [name, content] : plan.documents)
            zip.addBuffer(name, content);
        for (const auto& entry : plan.files)
            zip.addFileFromDisk("mods/" + entry.filename, mods_dir / entry.filename);
        zip.close("MCTools");
    }
    catch (...)
    {
        std::error_code ec;
        std::filesystem::remove_all(temp_dir, ec);
        throw;
    }

    return {archive.string(), std::move(plan.download_name), std::move(plan.warnings)};
}

void ModpackExporter::downloadMod(const ModsHelper::ModFileEntry& entry,
                                  const std::filesystem::path& mods_dir) const
{
    const auto target = mods_dir / entry.filename;

    cpr::Response response = cpr::Get(
        cpr::Url{entry.url},
        cpr::Header{{"User-Agent", "dscmoc-mctools/1.0 (MCTools modpack wizard)"}},
        cpr::Timeout{std::chrono::seconds(60)});

    if (response.status_code != 200)
        throw std::runtime_error("Не удалось скачать " + entry.title + " (HTTP " +
                                 std::to_string(response.status_code) + ")");

    std::ofstream output(target, std::ios::binary);
    if (!output)
        throw std::runtime_error("Не удалось записать " + target.string());

    // response.text holds the raw file bytes.
    const std::string& bytes = response.text;
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    if (!output)
        throw std::runtime_error("Ошибка записи " + target.string());
}

nlohmann::json ModpackExporter::buildMrpackIndex(
    const Request& request,
    const std::vector<ModsHelper::ModFileEntry>& files) const
{
    nlohmann::json index;
    index["formatVersion"] = 1;
    index["game"] = "minecraft";
    index["versionId"] = request.pack_name;
    index["name"] = request.pack_name;

    std::string summary = "Minecraft " + request.mc_version + ", " + request.loader;
    if (!request.loader_version.empty())
        summary += " " + request.loader_version;
    summary += " — " + std::to_string(files.size()) + " мод(и), собрано MCTools.";
    index["summary"] = summary;

    nlohmann::json json_files = nlohmann::json::array();
    for (const auto& entry : files)
    {
        nlohmann::json item;
        item["path"] = "mods/" + entry.filename;
        item["hashes"] = {{"sha1", entry.sha1}, {"sha512", entry.sha512}};
        item["env"] = {{"client", "required"}, {"server", "required"}};
        item["downloads"] = nlohmann::json::array({entry.url});
        item["fileSize"] = entry.size;
        json_files.push_back(std::move(item));
    }
    index["files"] = std::move(json_files);

    nlohmann::json dependencies;
    dependencies["minecraft"] = request.mc_version;
    if (!request.loader_version.empty())
    {
        if (request.loader == "fabric")
            dependencies["fabric-loader"] = request.loader_version;
        else if (request.loader == "forge")
            dependencies["forge"] = request.loader_version;
        else if (request.loader == "neoforge")
            dependencies["neoforge"] = request.loader_version;
    }
    index["dependencies"] = std::move(dependencies);

    return index;
}

std::string ModpackExporter::buildPakkuJson(
    const Request& request,
    const std::vector<ModsHelper::ModFileEntry>& files) const
{
    std::string out;
    out += "name = \"" + escapeToml(request.pack_name) + "\"\n";
    out += "game = \"minecraft\"\n";
    out += "minecraft-version = \"" + escapeToml(request.mc_version) + "\"\n";
    out += "mod-loader = \"" + escapeToml(request.loader) + "\"\n";
    if (!request.loader_version.empty())
        out += "mod-loader-version = \"" + escapeToml(request.loader_version) + "\"\n";

    for (const auto& entry : files)
    {
        out += "\n[[files]]\n";
        out += "file = \"mods/" + escapeToml(entry.filename) + "\"\n";
        out += "hash = \"" + escapeToml(entry.sha1) + "\"\n";
        out += "hash-format = \"sha1\"\n";
        // The file is bundled in mods/; Pakku doesn't need to download it.
        out += "mode = \"manual\"\n";
        out += "url = \"" + escapeToml(entry.url) + "\"\n";
        out += "\n[files.metadata]\n";
        out += "mod-id = \"" + escapeToml(entry.mod_id) + "\"\n";
        out += "version-id = \"" + escapeToml(entry.version_id) + "\"\n";
        out += "title = \"" + escapeToml(entry.title) + "\"\n";
    }

    if (!request.java_args.empty())
    {
        out += "\n[java]\n";
        out += "args = \"" + escapeToml(request.java_args) + "\"\n";
    }

    return out;
}

std::string ModpackExporter::buildReadme(
    const Request& request,
    const std::vector<ModsHelper::ModFileEntry>& files) const
{
    std::string out;
    out += request.pack_name + "\n";
    out += "===========\n\n";
    out += "Minecraft: " + request.mc_version + "\n";
    out += "Загрузчик:  " + request.loader;
    if (!request.loader_version.empty())
        out += " " + request.loader_version;
    out += "\n";
    out += "Модов:      " + std::to_string(files.size()) + "\n\n";

    if (!request.java_args.empty())
    {
        out += "Аргументы Java\n----------------\n";
        out += request.java_args + "\n\n";
    }

    if (!request.instructions.empty())
    {
        out += "Инструкция\n-----------\n";
        out += request.instructions + "\n\n";
    }

    out += "Состав сборки\n--------------\n";
    for (const auto& entry : files)
    {
        out += "* " + entry.title + " — mods/" + entry.filename + "\n";
        if (!entry.url.empty())
            out += "  " + entry.url + "\n";
    }

    out += "\nСобрано MCTools.\n";
    return out;
}
