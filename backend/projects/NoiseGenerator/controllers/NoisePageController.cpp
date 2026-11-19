#include "NoisePageController.hpp"

#include <chrono>
#include <random>

NoisePageController::NoisePageController()
    : m_images_paths(std::filesystem::path(TEMP_FOLDER) / ".gen_images")
{
}

void NoisePageController::generate(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback,
                                   NOISEGENERATOR_NAMSEPACE::NoiseData&& noise_data) const
{
    if (!noise_data.valid())
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    drogon::HttpResponsePtr resp;
    try
    {
        auto image = NOISEGENERATOR_NAMSEPACE::NoiseRender(noise_data.t, noise_data).createImage();
        const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        const auto path = m_images_paths / (std::to_string(timestamp) + "-" +
                                           std::to_string(std::random_device{}()) + ".bmp");
        std::filesystem::create_directories(m_images_paths);

        noise::utils::WriterBMP writer;
        writer.SetSourceImage(image);
        writer.SetDestFilename(path.string());
        writer.WriteDestFile();

        resp = drogon::HttpResponse::newFileResponse(path.string(), "", drogon::CT_IMAGE_BMP, "", req);
    }
    catch (...)
    {
        resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k500InternalServerError);
    }
    callback(resp);
}

void NoisePageController::noise(const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& callback) const
{
    drogon::HttpViewData data;
    data.insert("title", "noise");
    auto resp = drogon::HttpResponse::newHttpViewResponse("NoisePageView", data, req);
    resp->setStatusCode(drogon::k200OK);
    resp->setContentTypeCode(drogon::CT_TEXT_HTML);
    callback(resp);
}
