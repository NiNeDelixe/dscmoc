#include "PrjTemplateController.hpp"

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