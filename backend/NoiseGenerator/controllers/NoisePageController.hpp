#pragma once
#ifndef CONTROLLERS_NOISEPAGECONTROLLER_HPP_
#define CONTROLLERS_NOISEPAGECONTROLLER_HPP_

#include "CoreDefines.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <drogon/HttpController.h>

#include "noise/NoiseRender.hpp"

namespace drogon
{
    template <>
    inline NOISEGENERATOR_NAMSEPACE::NoiseData fromRequest(const drogon::HttpRequest& req)
    {
        auto& para = req.getParameters();
        NOISEGENERATOR_NAMSEPACE::NoiseData noise{};
        if (para.size() != 7)
        {
            return noise;
        }
        if (!para.empty())
        {
            noise.size = std::stoi(para.at("size"));
            noise.offset_x = std::stoi(para.at("offsetX"));
            noise.offset_y = std::stoi(para.at("offsetY"));
            noise.octaves = std::stoi(para.at("octaves"));
            noise.frequency = std::stof(para.at("frequency"));
            noise.persistence = std::stof(para.at("persistence"));
            noise.t = std::stoll(para.at("t"));
        }
        return noise;
    };

};


class NoisePageController : public drogon::HttpController<NoisePageController>
{
public:
	NoisePageController();
	~NoisePageController() = default;

	METHOD_LIST_BEGIN
		ADD_METHOD_TO(NoisePageController::generate, "/noise/generate?size=${1}&offsetX=${2}&offsetY=${3}&octaves=${4}&frequency=${5}&persistence=${6}&t=${7}", drogon::Get);
		ADD_METHOD_TO(NoisePageController::noise, "/noise", drogon::Get);
	METHOD_LIST_END

	void generate(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback, 
        NOISEGENERATOR_NAMSEPACE::NoiseData&& noise_data) const;

	void noise(const drogon::HttpRequestPtr& req,
		drogon::AdviceCallback&& callback) const;

    void download(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback);

private:
    std::filesystem::path m_images_paths;
};


#endif // !CONTROLLERS_NOISEPAGECONTROLLER_HPP_