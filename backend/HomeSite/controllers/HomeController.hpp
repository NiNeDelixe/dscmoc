#pragma once
#ifndef CONTROLLERS_HOMECONTROLLER_HPP_
#define CONTROLLERS_HOMECONTROLLER_HPP_

#include <drogon/HttpSimpleController.h>

class HomeController : public drogon::HttpSimpleController<HomeController>
{
public:
	HomeController() = default;
	~HomeController() = default;

	virtual void asyncHandleHttpRequest(const drogon::HttpRequestPtr& req,
		drogon::AdviceCallback&& callback) override;

	PATH_LIST_BEGIN
		PATH_ADD("/", drogon::Get);
		PATH_ADD("/contacts", drogon::Get);
		PATH_ADD("/about", drogon::Get);
		PATH_ADD("/projects", drogon::Get);
	PATH_LIST_END

private:

};


#endif // !CONTROLLERS_HOMECONTROLLER_HPP_