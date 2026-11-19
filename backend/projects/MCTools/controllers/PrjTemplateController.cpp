#include "PrjTemplateController.hpp"

#include <algorithm>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace
{
    void respondWithError(drogon::AdviceCallback& callback,
                          std::string_view message,
                          drogon::HttpStatusCode status)
    {
        Json::Value body;
        body["error"] = std::string(message);

        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(status);
        callback(resp);
    }

    std::shared_ptr<Json::Value> requireJsonBody(const drogon::HttpRequestPtr& req)
    {
        auto json = req->getJsonObject();
        if (!json || !json->isObject())
            return nullptr;
        return json;
    }

    std::string readString(const Json::Value& node, const char* key)
    {
        return node.isMember(key) && node[key].isString() ? node[key].asString() : std::string{};
    }
}  // namespace

void PrjTemplateController::templates(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	drogon::HttpViewData data;
	data.insert("title", "templates");
	auto resp = drogon::HttpResponse::newHttpViewResponse("prj_template", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void PrjTemplateController::getMcVersions(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    auto json = requireJsonBody(req);
    if (!json)
    {
        respondWithError(callback, "Ожидается JSON-тело запроса", drogon::k400BadRequest);
        return;
    }

    const std::string loader = readString(*json, "loader");
    if (!ModsHelper::isKnownLoader(loader))
    {
        respondWithError(callback, "Неизвестный загрузчик модов: " + loader, drogon::k400BadRequest);
        return;
    }

    try
    {
        Json::Value versions(Json::arrayValue);
        for (const auto& version : MinecraftVersions::getInstance().getAllReleases())
        {
            Json::Value item;
            item["id"] = version.id;
            item["type"] = version.type;
            item["release_time"] = version.release_time;
            versions.append(std::move(item));
        }

        Json::Value result;
        result["loader"] = loader;
        result["versions"] = std::move(versions);

        callback(drogon::HttpResponse::newHttpJsonResponse(result));
    }
    catch (const std::exception& e)
    {
        respondWithError(callback, std::string("Не удалось получить версии Minecraft: ") + e.what(),
                         drogon::k503ServiceUnavailable);
    }
}

void PrjTemplateController::getLoaderVersions(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    auto json = requireJsonBody(req);
    if (!json)
    {
        respondWithError(callback, "Ожидается JSON-тело запроса", drogon::k400BadRequest);
        return;
    }

    const std::string loader = readString(*json, "loader");
    const std::string mcVersion = readString(*json, "mc_version");

    if (!ModsHelper::isKnownLoader(loader) || mcVersion.empty())
    {
        respondWithError(callback, "Нужны поля loader и mc_version", drogon::k400BadRequest);
        return;
    }

    try
    {
        Json::Value versions(Json::arrayValue);

        for (const auto& entry : LoadersHelper::getInstance().getAllLoaderVersions(
                 mcVersion, LoadersHelper::loadersFromString(loader)))
        {
            Json::Value item;
            item["loader"] = entry.loader;
            item["version"] = entry.version;
            item["is_stable"] = entry.is_stable;
            item["mc_version"] = entry.mc_version;
            versions.append(std::move(item));
        }

        Json::Value result;
        result["versions"] = std::move(versions);

        callback(drogon::HttpResponse::newHttpJsonResponse(result));
    }
    catch (const std::exception& e)
    {
        respondWithError(callback, std::string("Не удалось получить версии загрузчика: ") + e.what(),
                         drogon::k503ServiceUnavailable);
    }
}

void PrjTemplateController::getCatalog(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    auto json = requireJsonBody(req);
    if (!json)
    {
        respondWithError(callback, "Ожидается JSON-тело запроса", drogon::k400BadRequest);
        return;
    }

    const std::string loader = readString(*json, "loader");
    if (!ModsHelper::isKnownLoader(loader))
    {
        respondWithError(callback, "Неизвестный загрузчик модов: " + loader, drogon::k400BadRequest);
        return;
    }

    std::vector<std::string> themes;
    if (json->isMember("themes") && (*json)["themes"].isArray())
    {
        for (const auto& node : (*json)["themes"])
        {
            if (node.isString())
                themes.push_back(node.asString());
        }
    }

    try
    {
        Json::Value mods(Json::arrayValue);
        for (const auto& item : ModCatalog::list(themes, loader))
        {
            Json::Value node;
            node["id"] = item.project_id;
            node["slug"] = item.slug;
            node["title"] = item.title;
            node["description"] = item.description;
            node["icon_url"] = item.icon_url;

            Json::Value themeList(Json::arrayValue);
            for (const auto& theme : item.themes)
                themeList.append(theme);
            node["themes"] = std::move(themeList);

            Json::Value loaderList(Json::arrayValue);
            for (const auto& name : item.loaders)
                loaderList.append(name);
            node["loaders"] = std::move(loaderList);

            mods.append(std::move(node));
        }

        Json::Value result;
        result["mods"] = std::move(mods);
        result["loader"] = loader;

        callback(drogon::HttpResponse::newHttpJsonResponse(result));
    }
    catch (const std::exception& e)
    {
        respondWithError(callback, std::string("Каталог модов недоступен: ") + e.what(),
                         drogon::k500InternalServerError);
    }
}

void PrjTemplateController::searchMods(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    auto json = requireJsonBody(req);
    if (!json)
    {
        respondWithError(callback, "Ожидается JSON-тело запроса", drogon::k400BadRequest);
        return;
    }

    const std::string loader = readString(*json, "loader");
    const std::string mcVersion = readString(*json, "mc_version");
    const std::string query = readString(*json, "query");

    if (!ModsHelper::isKnownLoader(loader))
    {
        respondWithError(callback, "Неизвестный загрузчик модов: " + loader, drogon::k400BadRequest);
        return;
    }
    if (query.empty())
    {
        respondWithError(callback, "Пустой поисковый запрос", drogon::k400BadRequest);
        return;
    }

    // windows.h defines min and max as macros.
    std::size_t limit = 30;
    if (json->isMember("limit") && (*json)["limit"].isUInt())
    {
        const auto requested = (*json)["limit"].asUInt();
        limit = requested > 50 ? 50 : requested;
    }
    std::size_t offset = 0;
    if (json->isMember("offset") && (*json)["offset"].isUInt())
        offset = (*json)["offset"].asUInt();

    try
    {
        Json::Value mods(Json::arrayValue);
        for (const auto& mod : ModsHelper::getInstance().search(query, loader, mcVersion, limit, offset))
        {
            Json::Value item;
            item["id"] = mod.id;
            item["slug"] = mod.slug;
            item["title"] = mod.title;
            item["description"] = mod.description;
            item["author"] = mod.author;
            item["icon_url"] = mod.icon_url;
            item["project_url"] = mod.project_url;
            item["downloads"] = Json::Int64(mod.downloads);
            mods.append(std::move(item));
        }

        Json::Value result;
        result["mods"] = std::move(mods);
        result["query"] = query;

        callback(drogon::HttpResponse::newHttpJsonResponse(result));
    }
    catch (const std::exception& e)
    {
        respondWithError(callback, std::string("Поиск модов не удался: ") + e.what(),
                         drogon::k503ServiceUnavailable);
    }
}

void PrjTemplateController::exportModpack(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    auto json = requireJsonBody(req);
    if (!json)
    {
        respondWithError(callback, "Ожидается JSON-тело запроса", drogon::k400BadRequest);
        return;
    }

    ModpackExporter::Request request;
    request.loader         = readString(*json, "loader");
    request.mc_version     = readString(*json, "mc_version");
    request.loader_version = readString(*json, "loader_version");
    request.pack_name      = readString(*json, "pack_name");
    request.java_args      = readString(*json, "java_args");
    request.instructions   = readString(*json, "instructions");

    // Invalid requests should return 400, not an export error (502).
    if (!ModsHelper::isKnownLoader(request.loader))
    {
        respondWithError(callback, "Неизвестный загрузчик модов: " + request.loader,
                         drogon::k400BadRequest);
        return;
    }
    if (request.mc_version.empty())
    {
        respondWithError(callback, "Не выбрана версия Minecraft", drogon::k400BadRequest);
        return;
    }

    if (!ModpackExporter::parseFormat(readString(*json, "format"), request.format))
    {
        respondWithError(callback, "Неизвестный формат экспорта", drogon::k400BadRequest);
        return;
    }

    const Json::Value& mods = (*json)["mods"];
    if (!mods.isArray() || mods.empty())
    {
        respondWithError(callback, "Не выбрано ни одного мода", drogon::k400BadRequest);
        return;
    }

    for (const auto& node : mods)
    {
        ModsHelper::ModEntry entry;
        entry.id          = readString(node, "id");
        entry.slug        = readString(node, "slug");
        entry.title       = readString(node, "title");
        entry.description = readString(node, "description");
        entry.author      = readString(node, "author");
        entry.icon_url    = readString(node, "icon_url");
        entry.project_url = readString(node, "project_url");
        if (node.isMember("downloads") && node["downloads"].isInt64())
            entry.downloads = node["downloads"].asInt64();

        if (entry.id.empty())
        {
            respondWithError(callback, "В списке модов есть запись без id", drogon::k400BadRequest);
            return;
        }
        request.mods.push_back(std::move(entry));
    }

    try
    {
        if (req->getParameter("prepare") == "1")
        {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
            resp->setBody(m_exporter.prepareBrowserExport(request).dump());
            callback(resp);
            return;
        }
        const ModpackExporter::Result result = m_exporter.build(request);

        // The second argument sets the attachment name; the fourth sets Content-Type.
        auto resp = drogon::HttpResponse::newFileResponse(
            result.archive_path, result.download_name,
            drogon::CT_APPLICATION_OCTET_STREAM, "", req);
        resp->setStatusCode(drogon::k200OK);

        if (!result.warnings.empty())
        {
            std::string joined;
            for (std::size_t i = 0; i < result.warnings.size(); ++i)
            {
                if (i > 0)
                    joined += "; ";
                joined += result.warnings[i];
            }
            resp->addHeader("X-MCTools-Skipped-Mods", joined);
        }

        LOG_INFO << "Собрана сборка " << result.download_name
                 << " (модов в архиве определено: " << result.warnings.size() << " пропущено)";
        callback(resp);
    }
    catch (const std::exception& e)
    {
        respondWithError(callback, std::string("Не удалось собрать сборку: ") + e.what(),
                         drogon::k502BadGateway);
    }
}
