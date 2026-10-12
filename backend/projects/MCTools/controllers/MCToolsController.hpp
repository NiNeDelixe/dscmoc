#pragma once
#ifndef CONTROLLERS_NOISE_PAGE_CONTROLLER_HPP_
#define CONTROLLERS_NOISE_PAGE_CONTROLLER_HPP_

#include "CoreDefines.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <drogon/HttpController.h>


class MCToolsController : public drogon::HttpController<MCToolsController>
{
public:
	MCToolsController();
	~MCToolsController() = default;

	METHOD_LIST_BEGIN
		ADD_METHOD_TO(MCToolsController::mctools, "/mctools", drogon::Get);
		ADD_METHOD_TO(MCToolsController::packs, "/mctools/packs", drogon::Get);
		ADD_METHOD_TO(MCToolsController::skin_viewer, "/mctools/skin_viewer", drogon::Get);
		ADD_METHOD_TO(MCToolsController::tools, "/mctools/tools", drogon::Get);
	METHOD_LIST_END

	void mctools(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void packs(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void skin_viewer(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void tools(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;

    //void download(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback);

private:
	
};



#endif // CONTROLLERS_NOISE_PAGE_CONTROLLER_HPP_
