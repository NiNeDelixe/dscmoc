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
	//resp->setContentTypeCodeAndCustomString(drogon::CT_TEXT_HTML, "text/html; charset=utf-8");
	callback(resp);
}
