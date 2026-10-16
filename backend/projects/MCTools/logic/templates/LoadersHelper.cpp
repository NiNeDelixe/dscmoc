#include "LoadersHelper.hpp"

std::vector<LoadersHelper::LoaderVersionEntry> LoadersHelper::getAllLoaderVersions(const std::string& mc_version, const Loaders& loader) const
{

    switch (loader)
    {
    case Loaders::FABRIC:
        return getAllFabricVersions(mc_version);
        break;
    case Loaders::FORGE:
        return getAllForgeVersions(mc_version);
        break;
    case Loaders::NEOFORGE:
        return getAllNeoForgeVersions(mc_version);
        break;

    case Loaders::ALL:
        break;
    
    default:
        break;
    }
    return std::vector<LoadersHelper::LoaderVersionEntry>();
}

std::vector<LoadersHelper::LoaderVersionEntry> LoadersHelper::getAllFabricVersions(const std::string &mc_version) const
{
    std::vector<LoaderVersionEntry> result;

    cpr::Response response = cpr::Get(cpr::Url{m_url_fabric} + "/" + mc_version);

    if (response.status_code != 200) 
    {
        if (!response.error.message.empty()) 
        {
            throw std::runtime_error(response.error.message);
        }
        else
        {
            throw std::runtime_error("error in fetching versions");
        }
    } 

    nlohmann::json json = nlohmann::json::parse(response.text);

    nlohmann::json versions = json;
    
    for (size_t i = 0; i < versions.size(); i++)
    {
        std::string version = versions[i]["loader"]["version"].template get<std::string>();
        bool is_stable = versions[i]["loader"]["stable"].template get<bool>();
        std::string mc_version_ = versions[i]["intermediary"]["version"].template get<std::string>();

        
        result.push_back(LoaderVersionEntry{"fabric", version, is_stable, mc_version_});
    }

    return result;
}

std::vector<LoadersHelper::LoaderVersionEntry> LoadersHelper::getAllNeoForgeVersions(const std::string &mc_version) const
{
    return std::vector<LoaderVersionEntry>();
}

std::vector<LoadersHelper::LoaderVersionEntry> LoadersHelper::getAllForgeVersions(const std::string &mc_version) const
{
    return std::vector<LoaderVersionEntry>();
}
