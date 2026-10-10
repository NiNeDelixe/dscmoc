#include "NoiseMaker.hpp"

void NOISEGENERATOR_NAMSEPACE::NoiseMaker::generate(const NoiseData& data)
{
	m_noise_module.SetSeed(m_seed);

	m_width = data.size;
	m_height = data.size;

	//m_x_bound = data.offset_x;
	//m_z_bound = data.offset_y;

	utils::NoiseMap heightMap;
	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(m_noise_module);
	heightMapBuilder.SetDestNoiseMap(m_noise_map);
	heightMapBuilder.SetDestSize(m_width, m_height);
	try
	{
		heightMapBuilder.SetBounds(data.offset_x, data.offset_x + m_x_bound, data.offset_y, data.offset_y + m_z_bound);
	}
	catch (const noise::ExceptionInvalidParam& exc)
	{
		LOG_ERROR << "Invalid params";
	}
	
	heightMapBuilder.Build();
}
