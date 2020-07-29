#include <iostream>
#include <assert.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

#include "web.h"

class TestCall
{
public:
	web::HttpResponse Home(const web::UrlParam& _params)
	{
		std::cout << "Home函数触发" << std::endl;
	
		return web::View("home/index.html");
	}
	
	web::HttpResponse TestPost(const web::UrlParam& _params)
	{
		std::cout << "TestPost触发" << std::endl;
	
		std::cout << "a:" << _params["a"].ToString() << std::endl;
		std::cout << "b:" << _params["b"].ToString() << std::endl;
	
		for(int i = 0; i < _params["c"]["test"].GetArraySize(); i++)
		{
			std::cout << "c[test][" << i << "] = " << _params["c"]["test"][i].ToString() << std::endl;
		}
		std::cout << "c[val] = " << _params["c"]["val"].ToString() << std::endl;
		for(int i = 0; i < _params["d"].GetArraySize(); i++)
		{
			std::cout << "d[" << i << "] = " << _params["d"][i].ToString() << std::endl;
		}
		
	
		return web::Json("success！测试中文!成功!");
	}


	web::HttpResponse Test(const web::UrlParam& _params)
	{
		std::cout << "in TestCall " << std::endl;
		return web::Json("6666");
	}
};

void TestWebSocket(const char* _data, size_t _len)
{
	std::string content(_data, _len);
	std::cout << "TestWebsocket called! data:" << content << " size:" << content.size() << std::endl;
}

int main(int _argc, char* _argv[])
{
	if(_argc != 3)
	{
		std::cout << "only have 2 argument! frist is ipaddress, second is port." << std::endl;
		return -1;
	}

	const char* ip = _argv[1];
	const char* port = _argv[2];


	std::unique_ptr<web::Router> test(new web::Router);

	static TestCall temp;

	test->RegisterUrl("GET", "/", &TestCall::Home, &temp);
	test->RegisterUrl("POST", "/test/post", &TestCall::TestPost, &temp);
	test->RegisterUrl("GET", "/Test", &TestCall::Test, &temp);
	test->RegisterWebSocket("/chat", &TestWebSocket);

	web::HttpServer server(std::move(test));

	server.Listen(ip, std::stoi(port));

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
