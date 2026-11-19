#ifndef CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_
#define CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_

#include "CoreDefines.hpp"

#include <string>
#include <vector>

#include <drogon/HttpController.h>

#include "projects/MCTools/logic/templates/LoadersHelper.hpp"
#include "projects/MCTools/logic/templates/MinecraftVersions.hpp"
#include "projects/MCTools/logic/templates/ModCatalog.hpp"
#include "projects/MCTools/logic/templates/ModpackExporter.hpp"
#include "projects/MCTools/logic/templates/ModsHelper.hpp"

class PrjTemplateController : public drogon::HttpController<PrjTemplateController>
{
public:
	PrjTemplateController() = default;
	~PrjTemplateController() = default;

	METHOD_LIST_BEGIN
		ADD_METHOD_TO(PrjTemplateController::templates, "/mctools/templates", drogon::Get);
		ADD_METHOD_TO(PrjTemplateController::getMcVersions, "/mctools/api/modpack/versions/mc", drogon::Post);
		ADD_METHOD_TO(PrjTemplateController::getLoaderVersions, "/mctools/api/modpack/versions/loaders", drogon::Post);
		ADD_METHOD_TO(PrjTemplateController::getCatalog, "/mctools/api/modpack/catalog", drogon::Post);
		ADD_METHOD_TO(PrjTemplateController::searchMods, "/mctools/api/modpack/mods/search", drogon::Post);
		ADD_METHOD_TO(PrjTemplateController::exportModpack, "/mctools/api/modpack/export", drogon::Post);
	METHOD_LIST_END

	void templates(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void getMcVersions(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void getLoaderVersions(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void getCatalog(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void searchMods(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;
	void exportModpack(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const;

private:
	ModpackExporter m_exporter;
};

#endif // CONTROLLERS_PRJ_TEMPLATE_CONTROLLER_HPP_