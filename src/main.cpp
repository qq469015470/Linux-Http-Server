#include "web.h"

#include "ExceptionHandler.hpp" // for log stacktrace line number

#include "WareHouseService.hpp"
#include "ItemInventoryService.hpp"
#include "MaterialService.hpp"
#include "CheckService.hpp"


#include <thread>
#include <chrono>
#include <netdb.h>
#include <filesystem>

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

	static inline DateTime Now()
	{
		DateTime now;

		now.tp = std::chrono::system_clock::now();

		return now;
	}

	inline DateTime AddYears(int _years)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60 * 24 * 365>>(_years);

		return temp;
	}

	inline DateTime AddMonths(int _months)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60 * 24 * 30>>(_months);

		return temp;
	}

	inline DateTime AddDays(int _days)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60 * 24>>(_days);

		return temp;
	}

	inline DateTime AddHours(int _hours)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60 * 60>>(_hours);

		return temp;
	}

	inline DateTime AddMinutes(int _minutes)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<60>>(_minutes);

		return temp;
	}

	inline DateTime AddSeconds(int _seconds)
	{
		DateTime temp;

		temp = *this;
		temp.tp += std::chrono::duration<int, std::ratio<1>>(_seconds);

		return temp;
	}

	inline std::string ToString() const
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

	//std::cout << "data:" << std::endl;
	//std::cout << temp["data"].ToJson() << std::endl;

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
		try
		{
			WareHouseService wareHouseService;

			std::vector<WareHouse> warehouses = wareHouseService.GetWareHouses();

			web::JsonObj result;
			for(const auto& item: warehouses)
			{
				web::JsonObj temp;

				temp["id"] = item.id;
				temp["name"] = item.name;

				result.Push(std::move(temp));
			}

			return web::Json(result);
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse Add(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			WareHouseService wareHouseService;

			wareHouseService.AddWareHouse(_params["_wareHouse"]["name"].ToString());

			return JsonData(JsonDataCode::Success, nullptr, "添加成功");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}
};

class ItemInventoryController
{
public:
	web::HttpResponse Add(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			ItemInventoryService itemInventoryService;
			MysqlService mysqlService;
	       		
			std::vector<std::string> sqlCmds;
	       
			const std::string itemInventoryId = mysqlService.GetUUID();

			sqlCmds	= itemInventoryService.GetAddItemInventorySql
								(
									itemInventoryId,
									_params["_materialName"].ToString(), 
									_params["_itemInventory"]["wareHouseId"].ToString(), 
									std::stod(_params["_itemInventory"]["cost"].ToString())
								);

			for(const auto& item: itemInventoryService.GetCheckInSql(itemInventoryId, std::stod(_params["_itemInventory"]["stock"].ToString()), false))
			{
				sqlCmds.emplace_back(item);
			}
			
			mysqlService.ExecuteCommandWithTran(sqlCmds);

			return JsonData(JsonDataCode::Success, nullptr, "添加成功");	
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse Edit(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			ItemInventoryService itemInventoryService;

			std::vector<std::string> sqlCmds;

			sqlCmds = itemInventoryService.GetEditItemInventorySql
					(
					 	_params["_itemInventoryId"].ToString(),
						_params["_name"].ToString(),
						std::stod(_params["_price"].ToString())
					);


			MysqlService mysqlService;

			mysqlService.ExecuteCommandWithTran(sqlCmds);

			return JsonData(JsonDataCode::Success, nullptr, "修改成功");	
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse Get(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			ItemInventoryService itemInventoryService;

			std::vector<ItemInventoryView> view = itemInventoryService.GetByWareHouseId(_params["_houseId"].ToString());

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

			//std::cout << "will response:" << std::endl;
			//std::cout << data.ToJson() << std::endl;
			
			return JsonData(JsonDataCode::Success, &data, "");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse CheckIn(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			const std::string itemInventoryId = _params["_itemInventoryId"].ToString();
			const double number = std::stod(_params["_number"].ToString());

			ItemInventoryService itemInventoryService; 

			const std::vector<std::string> sqlCmds = itemInventoryService.GetCheckInSql(itemInventoryId, number);

			MysqlService mysqlService;

			mysqlService.ExecuteCommandWithTran(sqlCmds);

			return JsonData(JsonDataCode::Success, nullptr, "");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse CheckOut(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			const std::string itemInventoryId = _params["_itemInventoryId"].ToString();
			const double number = std::stod(_params["_number"].ToString());

			ItemInventoryService itemInventoryService; 

			const std::vector<std::string> sqlCmds = itemInventoryService.GetCheckOutSql(itemInventoryId, number);

			MysqlService mysqlService;

			mysqlService.ExecuteCommandWithTran(sqlCmds);

			return JsonData(JsonDataCode::Success, nullptr, "");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse Contains(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			ItemInventoryService itemInventoryService;

			const std::vector<Material> materials = itemInventoryService.GetContainsMaterialName(_params["_houseId"].ToString(), _params["_materialName"].ToString());

			web::JsonObj data;
			for(const auto& item: materials)
			{
				web::JsonObj temp;

				temp["id"] = item.id;
				temp["name"] = item.name;

				data.Push(std::move(temp));
			}

			return JsonData(JsonDataCode::Success, &data, "");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}
};

class CheckController
{
public:
	web::HttpResponse Get(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			DateTime start(DateTime::Convert(_params["_date"].ToString()));
			DateTime end(start.AddDays(1));

			CheckService checkService;	

			const std::vector<CheckView> view = checkService.GetCheck
								(
									_params["_houseId"].ToString(),
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
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse GetDetail(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			DateTime start(DateTime::Convert(_params["_date"].ToString()));
			DateTime end(start.AddDays(1));
	
			CheckService checkService;
	
			const std::vector<CheckDetailView> view = checkService.GetCheckDetail
									(
										_params["_itemInventoryId"].ToString(),
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
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse CancelCheckIn(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			CheckService checkService;

			const std::vector<std::string> sql = checkService.GetCancelCheckInSql(_params["_id"].ToString());	

			MysqlService mysqlService;

			mysqlService.ExecuteCommandWithTran(sql);

			return JsonData(JsonDataCode::Success, nullptr, "");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse CancelCheckOut(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			CheckService checkService;

			const std::vector<std::string> sql = checkService.GetCancelCheckOutSql(_params["_id"].ToString());

			MysqlService mysqlService;

			mysqlService.ExecuteCommandWithTran(sql);

			return JsonData(JsonDataCode::Success, nullptr, "");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}
	}

	web::HttpResponse GetNote(const web::UrlParam& _params, const web::HttpHeader& _header)
	{
		try
		{
			CheckService checkService;

			const auto views = checkService.GetCheckNoteView(_params["_wareHouseId"].ToString(), _params["_date"].ToString());

			web::JsonObj result;
			for(const auto& item: views)
			{
				web::JsonObj temp;

				temp["id"] = item.id;
				temp["wareHouseId"] = item.wareHouseId;
				temp["name"] = item.name;
				temp["price"] = item.price;
				temp["stock"] = item.stock;

				result.Push(std::move(temp));
			}

			return JsonData(JsonDataCode::Success, &result, "");
		}
		catch(const std::logic_error& _ex)
		{
			return JsonData(JsonDataCode::Error, nullptr, _ex.what());
		}

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

	MysqlService::SetDataBase("dangkou_prod");

	std::unique_ptr<web::Router> router(new web::Router);

	static HomeController homeController;
	static WareHouseController wareHouseController;
	static ItemInventoryController itemInventoryController;
	static CheckController checkController;

	router->SetPage404Response([](const web::UrlParam& _params, const web::HttpHeader& _header){ return web::View("home/vue.html");});

	router->RegisterUrl("GET", "/", std::bind(&HomeController::Index, &homeController, std::placeholders::_1, std::placeholders::_2));

	router->RegisterUrl("GET", "/WareHouse/Get", std::bind(&WareHouseController::Get, &wareHouseController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("POST", "/WareHouse/Add", std::bind(&WareHouseController::Add, &wareHouseController, std::placeholders::_1, std::placeholders::_2));
	
	router->RegisterUrl("POST", "/ItemInventory/Add", std::bind(&ItemInventoryController::Add, &itemInventoryController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("POST", "/ItemInventory/Edit", std::bind(&ItemInventoryController::Edit, &itemInventoryController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("GET", "/ItemInventory/Get", std::bind(&ItemInventoryController::Get, &itemInventoryController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("POST", "/ItemInventory/CheckIn", std::bind(&ItemInventoryController::CheckIn, &itemInventoryController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("POST", "/ItemInventory/CheckOut", std::bind(&ItemInventoryController::CheckOut, &itemInventoryController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("GET", "/ItemInventory/Contains", std::bind(&ItemInventoryController::Contains, &itemInventoryController, std::placeholders::_1, std::placeholders::_2));

	router->RegisterUrl("GET", "/Check/Get", std::bind(&CheckController::Get, &checkController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("GET", "/Check/GetDetail", std::bind(&CheckController::GetDetail, &checkController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("POST", "/Check/CancelCheckIn", std::bind(&CheckController::CancelCheckIn, &checkController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("POST", "/Check/CancelCheckOut", std::bind(&CheckController::CancelCheckOut, &checkController, std::placeholders::_1, std::placeholders::_2));
	router->RegisterUrl("GET", "/Check/GetNote", std::bind(&CheckController::GetNote, &checkController, std::placeholders::_1, std::placeholders::_2));

	web::HttpServer server(std::move(router));

	server.UseSSL(false);

	server.SetHttpResponseCallBack([](const web::HttpRequest& _request, const web::HttpResponse& _response)
	{
		if(_response.GetStateCode() == 500)
		{
			const DateTime datetime = DateTime::Now();
			const std::string logPath = "log/";

			if(!std::filesystem::exists(std::filesystem::path(logPath)))
			{
				if(!std::filesystem::create_directory(logPath))
				{
					throw std::runtime_error("can not create log dir");
				}
			}

			const std::string threadId = std::to_string(std::hash<std::thread::id>()(std::this_thread::get_id()));
			const std::string logfileName = datetime.ToString() + std::string("_") + threadId + ".txt";

			std::fstream file(logPath + logfileName, std::ios::out);
			if(!file.is_open())
			{
				throw std::runtime_error("can not open file");
			}

			file << "***post***" << std::endl;

			std::string temp;

			temp += _request.GetType() + " ";
			temp += _request.GetUrl();
		       	if(_request.GetQueryString() != "")
				temp += "?" + _request.GetQueryString();

			temp += " ";
			temp += _request.GetVersion();
			temp += "\n";

			for(const auto& item: _request.GetHeader().GetHttpAttrs())
			{
				temp += item.GetKey() + ": " + item.GetValue() + "\n";
			}

			temp += "\n";

			temp += std::string(_request.GetBody(), _request.GetBodyLen());

			file << temp << std::endl;; 
			file << "**********" << std::endl;

			file << "***backtrace***" << std::endl;
			file << "thread id:" << std::this_thread::get_id() << std::endl;
			file << ExceptionHandler::GetLastStackTrace() << std::endl;
			file << "***************" << std::endl;

			file.close();
		}
	});

	server.Listen(ip, std::stoi(port));

	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
