#pragma once
#ifndef NOISE_NOISERENDER_HPP_
#define NOISE_NOISERENDER_HPP_

#include "CoreDefines.hpp"

#include <noise/noise.h>
#include <noise/noiseutils.h>

#include <trantor/utils/Logger.h>

#include "NoiseMaker.hpp"

namespace NOISEGENERATOR_NAMSEPACE
{
    class NoiseRender
    {
    public:
        NoiseRender(const long long& seed, const NoiseData& data)
            : m_noise_maker(seed), m_noise_data(data)
        {
        }
        ~NoiseRender() = default;

    public:
        noise::utils::Image createImage();


    private:
        noise::utils::RendererImage m_renderer;
        NoiseMaker m_noise_maker;
        NoiseData m_noise_data;
    };
}


#endif // !NOISE_NOISERENDER_HPP_