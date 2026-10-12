#pragma once
#ifndef NOISE_NOISEMAKER_HPP_
#define NOISE_NOISEMAKER_HPP_

#include "CoreDefines.hpp"

#include <optional>

#include <noise/noise.h>
#include <noise/module/perlin.h>
#include <noise/noiseutils.h>

#include <drogon/drogon.h>

namespace NOISEGENERATOR_NAMSEPACE
{
	struct NoiseData
	{
		size_t size;
		size_t offset_x;
		size_t offset_y;
		size_t octaves;
		float frequency;
		float persistence;
		long long t;
	};

	class NoiseMaker
	{
	public:
		NoiseMaker(const long long& seed)
			: m_seed(seed) {
		}
		~NoiseMaker() = default;

	public:
		void generate(const NoiseData& data);

	public:
		void setWidth(const int& value) { this->m_width = value; }
		void setHeight(const int& value) { this->m_height = value; }
		void setZBound(const double& value) { this->m_z_bound = value; }
		void setXBound(const double& value) { this->m_x_bound = value; }

		const noise::utils::NoiseMap& getNoiseMap() const { return m_noise_map; }
		const noise::module::Perlin& getModule() const { return m_noise_module; }

	private:
		long long m_seed = 0;
		int m_width = 512;
		int m_height = 512;
		double m_z_bound = 10;
		double m_x_bound = 10;

		noise::module::Perlin m_noise_module;
		noise::utils::NoiseMap m_noise_map;
	};
}

#endif // NOISE_NOISEMAKER_HPP_