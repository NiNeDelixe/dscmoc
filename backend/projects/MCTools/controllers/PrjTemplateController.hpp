#ifndef CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_
#define CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_

#include "CoreDefines.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <drogon/HttpController.h>


class PrjTemplateController : public drogon::HttpController<PrjTemplateController>
{
public:
	PrjTemplateController();
	~PrjTemplateController() = default;

	METHOD_LIST_BEGIN
		ADD_METHOD_TO(PrjTemplateController::templates, "/mctools/templates", drogon::Get);
	METHOD_LIST_END

	void templates(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;


private:
	
};

#endif // CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_
