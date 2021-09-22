#include "web.h"

#include "WareHouseService.hpp"
#include "ItemInventoryService.hpp"
#include "MaterialService.hpp"
#include "CheckService.hpp"

#include <thread>
#include <chrono>
#include <netdb.h>

class DateTime
{
private:
	//static constexpr char errorMsg[] = "转换失败";
	std::chrono::system_clock::time_point tp;

public:
	DateTime()
	{

	}

	static DateTime Convert(std::string_view _str)
	{
		/*if (_str[4] != '-' || _str[7] != '-' || _str[10] != ' ' || _str[13] != ':' || _str[16] != ':')
			throw std::exception(DateTime::errorMsg);

		for (int i = 0; i <= 3; i++)
			if (_str[i] < '0' || _str[i] > '9')
				throw std::exception(DateTime::errorMsg);

		for (int i = 5; i <= 6; i++)
			if (_str[i] < '0' || _str[i] > '9')
				throw std::exception(DateTime::errorMsg);

		for (int i = 8; i <= 9; i++)
			if (_str[i] < '0' || _str[i] > '9')
				throw std::exception(DateTime::errorMsg);

		for (int i = 11; i <= 12; i++)
			if (_str[i] < '0' || _str[i] > '9')
				throw std::exception(DateTime::errorMsg);

		for (int i = 14; i <= 15; i++)
			if (_str[i] < '0' || _str[i] > '9')
				throw std::exception(DateTime::errorMsg);

		for (int i = 17; i <= 18; i++)
			if (_str[i] < '0' || _str[i] > '9')
				throw std::exception(DateTime::errorMsg);*/

		DateTime date;
		time_t t;
		tm s = {};

		sscanf(_str.data(), "%d-%d-%d %d:%d:%d", &s.tm_year, &s.tm_mon, &s.tm_mday, &s.tm_hour, &s.tm_min, &s.tm_sec);

		s.tm_year -= 1900;
		s.tm_mon -= 1;

		if (s.tm_hour < 0)
			s.tm_hour = 0;
		if (s.tm_min < 0)
			s.tm_min = 0;
		if (s.tm_sec < 0)
			s.tm_sec = 0;

		t = std::mktime(&s);
		if (t == -1)
			throw std::runtime_error("转换失败");

		date.tp = std::chrono::system_clock::from_time_t(t);

		return date;
	}

	static DateTime Now()
	{
		DateTime now;

		now.tp = std::chrono::system_clock::now();

		return now;
	}

	DateTime AddYears(int _years)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60 * 24 * 365>>(_years);

		return temp;
	}

	DateTime AddMonths(int _months)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60 * 24 * 30>>(_months);

		return temp;
	}

	DateTime AddDays(int _days)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60 * 24>>(_days);

		return temp;
	}

	DateTime AddHours(int _hours)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60>>(_hours);

		return temp;
	}

	DateTime AddMinutes(int _minutes)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60>>(_minutes);

		return temp;
	}

	DateTime AddSeconds(int _seconds)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<1>>(_seconds);

		return temp;
	}

	std::string ToString()
	{
		time_t temp;
		std::tm time;

		temp = std::chrono::system_clock::to_time_t(this->tp);
		time = *std::localtime(&temp);

		std::stringstream ss;

		ss << std::put_time(&time, "%Y-%m-%d %H:%M:%S");
		return ss.str();
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

	temp["msg"] = _msg;

	std::cout << "data:" << std::endl;
	std::cout << temp["data"].ToJson() << std::endl;

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

		for(const auto& item: itemInventoryService.GetCheckInSql(itemInventoryId, std::stod(_params["_itemInventory"]["stock"].ToString()), false))
		{
			sqlCmds.emplace_back(item);
		}
		
		mysqlService.ExecuteCommandWithTran(sqlCmds);

		return JsonData(JsonDataCode::Success, nullptr, "添加成功");	
	}


	web::HttpResponse Edit(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		ItemInventoryService itemInventoryService;

		std::vector<std::string> sqlCmds;

		sqlCmds = itemInventoryService.GetEditItemInventorySql
				(
				 	std::stoi(_params["_itemInventoryId"].ToString()),
					_params["_name"].ToString(),
					std::stod(_params["_price"].ToString())
				);


		MysqlService mysqlService;

		mysqlService.ExecuteCommandWithTran(sqlCmds);

		return JsonData(JsonDataCode::Success, nullptr, "修改成功");	
	}

	web::HttpResponse Get(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		ItemInventoryService itemInventoryService;

		std::vector<ItemInventoryView> view = itemInventoryService.GetByWareHouseId(std::stoi(_params["_houseId"].ToString()));

		web::JsonObj data;
		for(const auto& item: view)
		{
			web::JsonObj temp;

			temp["id"] = item.id;
			temp["wareHouseId"] = item.wareHouseId;
			temp["materialId"] = item.materialId;
			temp["name"] = item.name;
			temp["price"] = item.price;
			temp["stock"] = item.stock;

			data.Push(std::move(temp));
		}

		std::cout << "will response:" << std::endl;
		std::cout << data.ToJson() << std::endl;
		
		return JsonData(JsonDataCode::Success, &data, "");
	}

	web::HttpResponse CheckIn(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		const int itemInventoryId = std::stoi(_params["_itemInventoryId"].ToString());
		const double number = std::stod(_params["_number"].ToString());

		ItemInventoryService itemInventoryService; 

		const std::vector<std::string> sqlCmds = itemInventoryService.GetCheckInSql(itemInventoryId, number);

		MysqlService mysqlService;

		mysqlService.ExecuteCommandWithTran(sqlCmds);

		return JsonData(JsonDataCode::Success, nullptr, "");
	}

	web::HttpResponse CheckOut(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		const int itemInventoryId = std::stoi(_params["_itemInventoryId"].ToString());
		const double number = std::stod(_params["_number"].ToString());

		ItemInventoryService itemInventoryService; 

		const std::vector<std::string> sqlCmds = itemInventoryService.GetCheckOutSql(itemInventoryId, number);

		MysqlService mysqlService;

		mysqlService.ExecuteCommandWithTran(sqlCmds);

		return JsonData(JsonDataCode::Success, nullptr, "");
	}
};

class CheckController
{
public:
	web::HttpResponse Get(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		DateTime start(DateTime::Convert(_params["_date"].ToString()));
		DateTime end(start.AddDays(1));

		CheckService checkService;	

		const std::vector<CheckView> view = checkService.GetCheck
							(
								std::stoi(_params["_houseId"].ToString()),
								start.ToString(),
								end.ToString()
							);

		web::JsonObj result;

		result.SetArrayNull();
		for(const auto& item: view)
		{
			web::JsonObj temp;

			temp["id"] = item.id;
			temp["name"] = item.name;
			temp["checkIn"] = item.checkIn;
			temp["checkOut"] = item.checkOut;
			temp["cost"] = item.cost;

			result.Push(std::move(temp));
		}

		return JsonData(JsonDataCode::Success, &result, "");
	}

	web::HttpResponse GetDetail(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		DateTime start(DateTime::Convert(_params["_date"].ToString()));
		DateTime end(start.AddDays(1));

		CheckService checkService;

		const std::vector<CheckDetailView> view = checkService.GetCheckDetail
								(
									std::stoi(_params["_itemInventoryId"].ToString()),
									start.ToString(),
									end.ToString()
								);

		web::JsonObj result;

		result.SetArrayNull();
		for(const auto& item: view)
		{
			web::JsonObj temp;

			temp["id"] = item.id;
			temp["number"] = item.number;
			temp["time"] = item.time;

			result.Push(std::move(temp));
		}

		return JsonData(JsonDataCode::Success, &result, "");
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
	static CheckController checkController;

	router->RegisterUrl("GET", "/", &HomeController::Index, &homeController);

	router->RegisterUrl("GET", "/WareHouse/Get", &WareHouseController::Get, &wareHouseController);
	router->RegisterUrl("POST", "/WareHouse/Add", &WareHouseController::Add, &wareHouseController);
	
	router->RegisterUrl("POST", "/ItemInventory/Add", &ItemInventoryController::Add, &itemInventoryController);
	router->RegisterUrl("POST", "/ItemInventory/Edit", &ItemInventoryController::Edit, &itemInventoryController);
	router->RegisterUrl("GET", "/ItemInventory/Get", &ItemInventoryController::Get, &itemInventoryController);
	router->RegisterUrl("POST", "/ItemInventory/CheckIn", &ItemInventoryController::CheckIn, &itemInventoryController);
	router->RegisterUrl("POST", "/ItemInventory/CheckOut", &ItemInventoryController::CheckOut, &itemInventoryController);

	router->RegisterUrl("GET", "/Check/Get", &CheckController::Get, &checkController);
	router->RegisterUrl("GET", "/Check/GetDetail", &CheckController::GetDetail, &checkController);

	web::HttpServer server(std::move(router));

	server.UseSSL(true);

	server.Listen(ip, std::stoi(port));

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
