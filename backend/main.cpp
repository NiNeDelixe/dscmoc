#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <dotenv.h>

#include "Users.h"

int main()
{
	dotenv::init();

	drogon::app().registerBeginningAdvice([]()
		{
			LOG_INFO << "Server is started in: http://" << drogon::app().getListeners().at(0).toIpPort() 
				<< " or http://localhost:" << drogon::app().getListeners().at(0).toPort();

			auto dbClient = drogon::app().getDbClient();
			// drogon::orm::Mapper<Users> mp(dbClient);
			// auto users = mp.orderBy(Users::Cols::_join_time).limit(25).offset(0).findAll();
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