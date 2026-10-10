#include "HomeController.hpp"

void HomeController::asyncHandleHttpRequest(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback)
{
	auto& para = req->getParameters();
	drogon::HttpViewData data;
	data.insert("title", "home");
	auto resp = drogon::HttpResponse::newHttpViewResponse("HomePageView", data, req);
	resp->setStatusCode(drogon::k200OK);
	resp->setContentTypeCode(drogon::CT_TEXT_HTML);
	//resp->setContentTypeCodeAndCustomString(drogon::CT_TEXT_HTML, "text/html; charset=utf-8");
	callback(resp);
}
