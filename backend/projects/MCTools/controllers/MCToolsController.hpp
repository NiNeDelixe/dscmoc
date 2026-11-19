#pragma once
#ifndef CONTROLLERS_MCTOOLS_CONTROLLER_HPP_
#define CONTROLLERS_MCTOOLS_CONTROLLER_HPP_

#include <drogon/HttpController.h>

class MCToolsController : public drogon::HttpController<MCToolsController>
{
public:
	MCToolsController() = default;
	~MCToolsController() = default;

	METHOD_LIST_BEGIN
		ADD_METHOD_TO(MCToolsController::mctools, "/mctools", drogon::Get);
		ADD_METHOD_TO(MCToolsController::packs, "/mctools/packs", drogon::Get);
		ADD_METHOD_TO(MCToolsController::skin_viewer, "/mctools/skin_viewer", drogon::Get);
		ADD_METHOD_TO(MCToolsController::tools, "/mctools/tools", drogon::Get);
		ADD_METHOD_TO(MCToolsController::apiTools, "/mctools/api/tools", drogon::Get);
	METHOD_LIST_END

	void mctools(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void packs(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void skin_viewer(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void tools(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;

	// Drogon doesn't serve CSV files by default.
	void apiTools(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
};

#endif // CONTROLLERS_MCTOOLS_CONTROLLER_HPP_