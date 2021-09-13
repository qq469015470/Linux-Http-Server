#include <netdb.h>

#include "web.h"

#include "WareHouseService.hpp"

#include <thread>
#include <chrono>

class TestCall
{
private:

public:
	web::HttpResponse Home(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		std::cout << "Home函数触发" << std::endl;
	
		return web::View("home/index.html");
	}
	
	web::HttpResponse TestPost(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		std::cout << "TestPost触发" << std::endl;
	
		std::cout << "a:" << _params["a"].ToString() << std::endl;
		std::cout << "b:" << _params["b"].ToString() << std::endl;
	
		for(size_t i = 0; i < _params["c"]["test"].GetArraySize(); i++)
		{
			std::cout << "c[test][" << i << "] = " << _params["c"]["test"][i].ToString() << std::endl;
		}
		std::cout << "c[val] = " << _params["c"]["val"].ToString() << std::endl;
		for(size_t i = 0; i < _params["d"].GetArraySize(); i++)
		{
			std::cout << "d[" << i << "] = " << _params["d"][i].ToString() << std::endl;
		}
		
	
		return web::Json("success！测试中文!成功!");
	}


	web::HttpResponse Test(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		std::cout << "in TestCall " << std::endl;
		return web::Json("6666");
	}

	web::HttpResponse TestNumber(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		std::cout << "a:" << _params["a"].ToString() << std::endl;
		std::cout << "b:" << _params["b"].ToString() << std::endl;

		std::this_thread::sleep_for(std::chrono::seconds(10));
		
		std::cout << "c:" << _params["c"].ToString() << std::endl;

		return web::Json("");
	}
};

void WebsocketOnConnect(web::Websocket* _websocket, const web::HttpHeader& _header)
{
	std::cout << "OnConnect id:" << _websocket->GetId() << std::endl;
}

void TestWebSocket(web::Websocket* _websocket, const char* _data, size_t _len)
{
	std::cout << "weboscketId" << _websocket->GetId() << std::endl;
	std::string content(_data, _len);
	std::cout << "TestWebsocket called! data:" << content << " size:" << content.size() << std::endl;

	struct User
	{
		int id;
		char name[10];
	};

	User user;
	const char* testuser = "tes tuser";

	user.id = 12345;
	std::copy(testuser, testuser + strlen(testuser), user.name);

	_websocket->SendText("hello websocket!");
	_websocket->SendByte(reinterpret_cast<char*>(&user), sizeof(user));
}

void WebsocketDisconnect(web::Websocket* _websocket)
{
	std::cout << "disconnect id:" << _websocket->GetId() << std::endl;
}

class Chat
{
public:
	void OnConnect(web::Websocket* _websocket, const web::HttpHeader& _header)
	{
		std::cout << "Chat Connect! " << std::endl;	
	}
	
	void OnMessage(web::Websocket* _websocket, const char* _data, size_t _len)
	{
		std::cout << "Chat OnMessage:" << std::string(_data, _len) << std::endl;
		_websocket->SendText("server recv");
	}

	void OnDisconnect(web::Websocket* _websocket)
	{
		std::cout << "Chat Disconnect!" << std::endl;
	}
};

class HomeController
{
private:
	WareHouseService wareHouseService; 

public:
	web::HttpResponse Index(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		return web::View("home/vue.html");
	}


};

enum JsonDataCode: int
{
	Success = 0,
	Error = -1,
};

web::HttpResponse JsonData(JsonDataCode _code, const web::JsonObj* const _data, std::string_view _msg)
{
	web::JsonObj temp;

	temp["code"] = static_cast<int>(_code);

	if(_data != nullptr)
		temp["data"] = web::JsonObj::ParseJson(_data->ToJson());
	else
		temp["data"].SetNull();

	temp["msg"] = _msg.data();

	return web::Json(temp);
}

class WareHouseController
{
private:

public:
	web::HttpResponse Get(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		WareHouseService wareHouseService;

		std::vector<WareHouse> warehouses = wareHouseService.GetWareHouses();

		web::JsonObj result;
		for(const auto& item: warehouses)
		{
			web::JsonObj temp;

			temp["id"] = std::to_string(item.id);
			temp["name"] = item.name;

			result.Push(std::move(temp));
		}

		return web::Json(result);
	}

	web::HttpResponse Add(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		WareHouseService wareHouseService;

		return JsonData(JsonDataCode::Error, nullptr, "1112");
	}
};

int main(int _argc, char* _argv[])
{
	
	//Test Client
	web::HttpClient client;

	const hostent* const host = gethostbyname("www.baidu.com");
	if(!host)
	{
		throw std::runtime_error("get host error");
	}

	const std::string url = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

	std::cout << "IP addr:" << url << std::endl;
	client.Connect(url);

	web::HttpResponse response = client.SendRequest("GET", "/");

	std::cout << std::string(response.GetContent(), response.GetContentSize()) << std::endl;
	
	//Test Server
	if(_argc != 3)
	{
		std::cout << "only have 2 argument! frist is ipaddress, second is port." << std::endl;
		return -1;
	}

	const char* ip = _argv[1];
	const char* port = _argv[2];


	std::unique_ptr<web::Router> test(new web::Router);

	static TestCall temp;
	static Chat chat;
	static HomeController homeController;
	static WareHouseController wareHouseController;

	test->RegisterUrl("GET", "/", &HomeController::Index, &homeController);
	test->RegisterUrl("GET", "/WareHouse/Get", &WareHouseController::Get, &wareHouseController);
	test->RegisterUrl("POST", "/WareHouse/Add", &WareHouseController::Add, &wareHouseController);


	test->RegisterUrl("POST", "/test/post", &TestCall::TestPost, &temp);
	test->RegisterUrl("POST", "/test/number", &TestCall::TestNumber, &temp);
	test->RegisterUrl("GET", "/Test", &TestCall::Test, &temp);
	test->RegisterWebsocket("/chat", &WebsocketOnConnect, &TestWebSocket, &WebsocketDisconnect);
	test->RegisterWebsocket("/asd", &Chat::OnConnect, &Chat::OnMessage, &Chat::OnDisconnect, &chat);

	web::HttpServer server(std::move(test));

	server.UseSSL(true);

	server.Listen(ip, std::stoi(port));

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
