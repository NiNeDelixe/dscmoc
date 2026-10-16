#include "PrjTemplateController.hpp"

PrjTemplateController::PrjTemplateController()
{
    m_mc_versions = MinecraftVersions::getInstance().getAllVersions();
}

void PrjTemplateController::templates(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    auto& para = req->getParameters();
	drogon::HttpViewData data;
	data.insert("title", "templates");
	auto resp = drogon::HttpResponse::newHttpViewResponse("prj_template", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void PrjTemplateController::getMcVersions(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    const auto json = req->getJsonObject();

    const std::unordered_set<std::string> themesAllowed{
        "optimization", "visual", "quality-of-life"
    };

    const std::unordered_set<std::string> loadersAllowed{
        "fabric", "forge", "neoforge"
    };

    if (!json || !(*json)["themes"].isArray() ||
        (*json)["themes"].empty() ||
        !(*json)["loader"].isString())
    {
        auto res = drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        callback(res);
        return;
    }

    const auto loader = (*json)["loader"].asString();

    if (loadersAllowed.find(loader) == loadersAllowed.end())
    {
        auto res = drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        callback(res);
        return;
    }

    for (const auto& theme : (*json)["themes"])
    {
        if (!theme.isString() ||
            themesAllowed.find(theme.asString()) == themesAllowed.end())
        {
            auto res = drogon::HttpResponse::newHttpResponse();
            res->setStatusCode(drogon::k400BadRequest);
            callback(res);
            return;
        }
    }

    Json::Value versions(Json::arrayValue);

    for (const auto& version : m_mc_versions)
    {
        if (version.type != "release")
            continue;

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

void PrjTemplateController::getLoaderVersions(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    const auto json = req->getJsonObject();

    if (!json || !(*json)["loader"].isString() ||
        !(*json)["mc_version"].isString())
    {
        auto res = drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        callback(res);
        return;
    }

    const auto loader = (*json)["loader"].asString();
    const auto mcVersion = (*json)["mc_version"].asString();

    std::vector<LoadersHelper::LoaderVersionEntry> entries;

    if (loader == "fabric")
        entries = LoadersHelper::getInstance().getAllFabricVersions(mcVersion);
    else if (loader == "forge")
        entries = LoadersHelper::getInstance().getAllForgeVersions(mcVersion);
    else if (loader == "neoforge")
        entries = LoadersHelper::getInstance().getAllNeoForgeVersions(mcVersion);
    else
    {
        auto res = drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        callback(res);
        return;
    }

    Json::Value versions(Json::arrayValue);

    for (const auto& entry : entries)
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
