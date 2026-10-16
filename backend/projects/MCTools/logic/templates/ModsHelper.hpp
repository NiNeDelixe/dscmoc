#ifndef TEMPLATES_MODS_HELPER_HPP_
#define TEMPLATES_MODS_HELPER_HPP_

#include "CoreDefines.hpp"

#include <string>
#include <vector>

#include <cpr/api.h>
#include <nlohmann/json.hpp>
#include <deluxe_lib/core/core.hpp>

class ModsHelper
{
    DL_DECLARE_CLASS(ModsHelper)

public:
    ~ModsHelper() = default;

public:
    struct ModEntry
    {
        /* data */
    };
    

public:


private:
    const std::string m_base_url = "https://api.modrinth.com/v2";
    const std::string m_projects_uri = "/project";
};

#endif // TEMPLATES_MODS_HELPER_HPP_
