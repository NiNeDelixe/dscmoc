#pragma once
#ifndef NOISE_NOISEMAKER_HPP_
#define NOISE_NOISEMAKER_HPP_

#include "CoreDefines.hpp"

#include <noise/noise.h>
#include <noise/module/perlin.h>
#include <noise/noiseutils.h>

namespace NOISEGENERATOR_NAMSEPACE
{
	struct NoiseData
	{
		int size;
		int offset_x;
		int offset_y;
		int octaves;
		float frequency;
		float persistence;
		long long t;

		bool valid() const
		{
			return size >= 20 && size <= 1000 && octaves >= 1 && octaves <= 8
				&& offset_x >= -1000 && offset_x <= 1000 && offset_y >= -1000 && offset_y <= 1000
				&& frequency >= 0.01f && frequency <= 0.2f
				&& persistence >= 0.1f && persistence <= 1.0f;
		}
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
