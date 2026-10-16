#ifndef CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_
#define CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_

#include "CoreDefines.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <unordered_set>

#include <drogon/HttpController.h>

#include "projects/MCTools/logic/templates/MinecraftVersions.hpp"
#include "projects/MCTools/logic/templates/LoadersHelper.hpp"

class PrjTemplateController : public drogon::HttpController<PrjTemplateController>
{
public:
	PrjTemplateController();
	~PrjTemplateController() = default;

	METHOD_LIST_BEGIN
		ADD_METHOD_TO(PrjTemplateController::templates, "/mctools/templates", drogon::Get);
		ADD_METHOD_TO(PrjTemplateController::getMcVersions, "/mctools/api/modpack/versions/mc", drogon::Post);
		ADD_METHOD_TO(PrjTemplateController::getLoaderVersions, "/mctools/api/modpack/versions/loaders", drogon::Post);
	METHOD_LIST_END

	void templates(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void getMcVersions(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;	
	void getLoaderVersions(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;	

private:
	std::vector<MinecraftVersions::VersionEntry> m_mc_versions;
};

#endif // CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_
