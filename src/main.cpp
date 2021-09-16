#include <netdb.h>

#include "web.h"

#include "WareHouseService.hpp"
#include "ItemInventoryService.hpp"

#include <thread>
#include <chrono>

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

	temp["msg"] = _msg;

	std::cout << "aa:" << std::endl;
	std::cout << _msg << std::endl;
	std::cout << "bb:" << std::endl;
	std::cout << temp["msg"].ToString() << std::endl;

	return web::Json(temp);
}

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

		wareHouseService.AddWareHouse(_params["_wareHouse"]["name"].ToString());

		return JsonData(JsonDataCode::Success, nullptr, "添加成功");
	}
};

class ItemInventoryController
{
public:
	web::HttpResponse Add(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		ItemInventoryService itemInventoryService;
	       	
		std::vector<std::string> sqlCmds;
	       
		sqlCmds	= itemInventoryService.GetAddItemInventorySql
							(
								_params["_materialName"].ToString(), 
								std::stoi(_params["_itemInventory"]["wareHouseId"].ToString()), 
								std::stod(_params["_itemInventory"]["cost"].ToString())
							);
		MysqlService mysqlService;

		const int& itemInventoryId = mysqlService.GetNextInsertId("itemInventory");

		for(const auto& item: itemInventoryService.GetCheckInSql(itemInventoryId, std::stod(_params["_itemInventory"]["stock"].ToString())))
		{
			sqlCmds.emplace_back(item);
		}
		
		mysqlService.ExecuteCommandWithTran(sqlCmds);

		return JsonData(JsonDataCode::Success, nullptr, "添加成功");	
	}
};

int main(int _argc, char* _argv[])
{	
	if(_argc != 3)
	{
		std::cout << "only have 2 argument! frist is ipaddress, second is port." << std::endl;
		return -1;
	}

	const char* ip = _argv[1];
	const char* port = _argv[2];


	std::unique_ptr<web::Router> router(new web::Router);

	static HomeController homeController;
	static WareHouseController wareHouseController;
	static ItemInventoryController itemInventoryController;

	router->RegisterUrl("GET", "/", &HomeController::Index, &homeController);

	router->RegisterUrl("GET", "/WareHouse/Get", &WareHouseController::Get, &wareHouseController);
	router->RegisterUrl("POST", "/WareHouse/Add", &WareHouseController::Add, &wareHouseController);
	
	router->RegisterUrl("POST", "/ItemInventory/Add", &ItemInventoryController::Add, &itemInventoryController);

	web::HttpServer server(std::move(router));

	server.UseSSL(true);

	server.Listen(ip, std::stoi(port));

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
