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

web::HttpResponse Home()
{
	std::cout << "Home函数触发" << std::endl;

	return web::View("home/index.html");
}

web::HttpResponse TestPost()
{
	std::cout << "TestPost触发" << std::endl;

	return web::Json("success！测试中文!成功!");
}

int main(int _argc, char* _argv[])
{
	//web::UrlParam temp;
	//
	//std::cout << temp[0] << std::endl;
	//web::UrlParam* asd = temp["123"];

	//return 0;


	if(_argc != 3)
	{
		std::cout << "only have 2 argument! frist is ipaddress, second is port." << std::endl;
		return -1;
	}

	const char* ip = _argv[1];
	const char* port = _argv[2];


	std::unique_ptr<web::Router> test(new web::Router);

	test->RegisterUrl("GET", "/", &Home);
	test->RegisterUrl("POST", "/test/post", &TestPost);

	web::HttpServer server(std::move(test));

	server.Listen(ip, std::stoi(port));

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
