#include "MinecraftVersions.hpp"

std::vector<MinecraftVersions::VersionEntry> MinecraftVersions::getAllVersions() const
{
    cpr::Response response = cpr::Get(cpr::Url{m_url});

    std::vector<VersionEntry> result;

    if (response.status_code == 200) 
    {
        nlohmann::json json = nlohmann::json::parse(response.text);

        nlohmann::json versions = json["versions"];
        
        for (size_t i = 0; i < versions.size(); i++)
        {
            std::string id = versions[i]["id"].template get<std::string>();
            std::string type = versions[i]["type"].template get<std::string>();
            std::string url = versions[i]["url"].template get<std::string>();
            std::string time = versions[i]["time"].template get<std::string>();
            std::string releaseTime = versions[i]["releaseTime"].template get<std::string>();
            
            result.push_back(VersionEntry{id, type, url, time, releaseTime});
        }
    } 
    else 
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
    return result;
}