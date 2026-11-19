#include "MCToolsController.hpp"

namespace
{
    struct ToolCard
    {
        const char* id;
        const char* title;
        const char* description;
        const char* url;
        const char* icon;
        bool only_additional;  // Only shown on /mctools/tools.
    };

    constexpr ToolCard kTools[] = {
        {"templates", "Конструктор модпака",
         "Подбор модов по Modrinth с фильтром по загрузчику и версии, экспорт в ZIP, .mrpack "
         "и проект Pakku.",
         "/mctools/templates", "file-text", false},
        {"packs", "Готовые сборки", "Подборки, которые можно скачать и запустить сразу.",
         "/mctools/packs", "box", false},
        {"skin_viewer", "Skin Viewer", "Просмотр скинов игроков.", "/mctools/skin_viewer",
         "user-round", false},
        {"tools", "Дополнительные инструменты", "Прочие утилиты для работы с модпаками.",
         "/mctools/tools", "settings", true},
    };
}

void MCToolsController::mctools(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	drogon::HttpViewData data;
	data.insert("title", "mctools");
	auto resp = drogon::HttpResponse::newHttpViewResponse("MCToolsView", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void MCToolsController::packs(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	drogon::HttpViewData data;
	data.insert("title", "packs");
	auto resp = drogon::HttpResponse::newHttpViewResponse("packs", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void MCToolsController::skin_viewer(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	drogon::HttpViewData data;
	data.insert("title", "skin_viewer");
	auto resp = drogon::HttpResponse::newHttpViewResponse("skin_viewer", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void MCToolsController::tools(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	drogon::HttpViewData data;
	data.insert("title", "tools");
	auto resp = drogon::HttpResponse::newHttpViewResponse("tools", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void MCToolsController::apiTools(const drogon::HttpRequestPtr &, drogon::AdviceCallback &&callback) const
{
	Json::Value all(Json::arrayValue);
	Json::Value additional(Json::arrayValue);

	for (const auto& tool : kTools)
	 {
        Json::Value item;
        item["id"] = tool.id;
        item["title"] = tool.title;
        item["description"] = tool.description;
        item["url"] = tool.url;
        item["icon"] = tool.icon;

        all.append(item);
        if (tool.only_additional)
            additional.append(std::move(item));
    }

	Json::Value result;
	result["tools"] = std::move(all);
	result["additional"] = std::move(additional);

	callback(drogon::HttpResponse::newHttpJsonResponse(result));
}