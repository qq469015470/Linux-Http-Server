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
	if(_argc != 3)
	{
		std::cout << "only have 2 argument! frist is ipaddress, second is port." << std::endl;
		return -1;
	}

	const char* ip = _argv[1];
	const char* port = _argv[2];


	std::unique_ptr<web::Router> test(new web::Router);

	test->RegisterUrl("GET", "/", { {"id", &typeid(int)}, {"test", &typeid(std::string)}, {"test2", &typeid(std::vector<std::string>)} }, &Home);
	test->RegisterUrl("POST", "/test/post", { {"a", &typeid(int)}, {"b", &typeid(std::string)} }, &TestPost);

	web::HttpServer server(std::move(test));

	server.Listen(ip, std::stoi(port));

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
