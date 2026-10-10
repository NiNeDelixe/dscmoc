#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <dotenv.h>

int main()
{
	dotenv::init();

	drogon::app().registerBeginningAdvice([]()
		{
			LOG_INFO << "Server is started in: http://" << drogon::app().getListeners().at(0).toIpPort() 
				<< " or http://localhost:" << drogon::app().getListeners().at(0).toPort();
		});

	drogon::app()
#ifndef NDEBUG
		.loadConfigFile("config.dev.json")
#else
		.loadConfigFile("config.json")
#endif // !NDEBUG
		.run();

	return 0;
}