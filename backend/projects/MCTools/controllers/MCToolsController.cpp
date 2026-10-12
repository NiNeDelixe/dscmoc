#include "MCToolsController.hpp"

MCToolsController::MCToolsController()
{

}

void MCToolsController::mctools(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
    auto& para = req->getParameters();
	drogon::HttpViewData data;
	data.insert("title", "mctools");
	auto resp = drogon::HttpResponse::newHttpViewResponse("MCToolsView", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void MCToolsController::packs(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	auto& para = req->getParameters();
	drogon::HttpViewData data;
	data.insert("title", "packs");
	auto resp = drogon::HttpResponse::newHttpViewResponse("packs", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void MCToolsController::skin_viewer(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	auto& para = req->getParameters();
	drogon::HttpViewData data;
	data.insert("title", "skin_viewer");
	auto resp = drogon::HttpResponse::newHttpViewResponse("skin_viewer", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}

void MCToolsController::tools(const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&callback) const
{
	auto& para = req->getParameters();
	drogon::HttpViewData data;
	data.insert("title", "tools");
	auto resp = drogon::HttpResponse::newHttpViewResponse("tools", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	callback(resp);
}
