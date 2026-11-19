#include "NoiseMaker.hpp"

void NOISEGENERATOR_NAMSEPACE::NoiseMaker::generate(const NoiseData& data)
{
	m_noise_module.SetSeed(static_cast<int>(m_seed));
	m_noise_module.SetOctaveCount(data.octaves);
	m_noise_module.SetFrequency(data.frequency);
	m_noise_module.SetPersistence(data.persistence);

	m_width = data.size;
	m_height = data.size;

	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(m_noise_module);
	heightMapBuilder.SetDestNoiseMap(m_noise_map);
	heightMapBuilder.SetDestSize(m_width, m_height);
	heightMapBuilder.SetBounds(data.offset_x, data.offset_x + m_x_bound, data.offset_y, data.offset_y + m_z_bound);
	heightMapBuilder.Build();
}
