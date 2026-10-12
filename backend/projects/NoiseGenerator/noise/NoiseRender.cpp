#include "NoiseRender.hpp"

noise::utils::Image NOISEGENERATOR_NAMSEPACE::NoiseRender::createImage()
{
	m_noise_maker.generate(m_noise_data);

	noise::utils::Image image;

	m_renderer.SetSourceNoiseMap(m_noise_maker.getNoiseMap());
	m_renderer.SetDestImage(image);
	m_renderer.ClearGradient();
	m_renderer.AddGradientPoint(-1.0000, noise::utils::Color(0, 0, 128, 255)); // deeps
	m_renderer.AddGradientPoint(-0.2500, noise::utils::Color(0, 0, 255, 255)); // shallow
	m_renderer.AddGradientPoint(0.0000, noise::utils::Color(0, 128, 255, 255)); // shore
	m_renderer.AddGradientPoint(0.0625, noise::utils::Color(240, 240, 64, 255)); // sand
	m_renderer.AddGradientPoint(0.1250, noise::utils::Color(32, 160, 0, 255)); // grass
	m_renderer.AddGradientPoint(0.3750, noise::utils::Color(224, 224, 0, 255)); // dirt
	m_renderer.AddGradientPoint(0.7500, noise::utils::Color(128, 128, 128, 255)); // rock
	m_renderer.AddGradientPoint(1.0000, noise::utils::Color(255, 255, 255, 255)); // snow
	m_renderer.EnableLight();
	m_renderer.SetLightContrast(3.0); // Triple the contrast
	m_renderer.SetLightBrightness(2.0); // Double the brightness

	try
	{
		m_renderer.Render();
	}
	catch (const noise::Exception& exc)
	{
		LOG_ERROR << "error during creating noise image";
	}
	

	return image;
}