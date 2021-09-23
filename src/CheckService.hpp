#pragma once 

#include "MysqlService.hpp"
#include "ItemInventoryService.hpp"

#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

struct CheckView
{
	int id;
	std::string name;
	double checkIn;
	double checkOut;
	double cost;
};

struct CheckDetailView
{
	int id;
	double number;	
	std::string time;
};

class CheckService
{
private:
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

public:
	std::vector<CheckView> GetCheck(const int& _wareHouseId, std::string_view _start, std::string_view _end)
	{
		std::vector<CheckView> result;

		const auto datatable = this->mysqlService.Query
			(
				"select itemInventory.*, material.name from itemInventory "
				"left join material on material.id = itemInventory.materialId "
				"where warehouseId = ?",
				_wareHouseId
			);

		for(const auto& item: datatable)
		{
			double checkInTotal(0);
			double checkOutTotal(0);

			const auto checkInData = this->mysqlService.Query
									(
										"select sum(number) as number from checkIn where itemInventoryId = ? and time between ? and ?",
										*reinterpret_cast<const int*>(item.at("id")->data()),
										_start,
										_end
									);
			if(checkInData.size() > 0)
			{
				checkInTotal += *reinterpret_cast<const double*>(checkInData.front().at("number")->data());
			}
			const auto checkOutData = this->mysqlService.Query
									(
										"select sum(number) as number from checkOut where itemInventoryId = ? and time between ? and ?",
										*reinterpret_cast<const int*>(item.at("id")->data()),
										_start,
										_end
									);
			if(checkOutData.size() > 0)
			{
				checkOutTotal += *reinterpret_cast<const double*>(checkOutData.front().at("number")->data());
			}

			if(checkInTotal != 0 || checkOutTotal != 0)
			{
				result.push_back
				({
					.id = *reinterpret_cast<const int*>(item.at("id")->data()),
					.name = item.at("name")->data(),
					.checkIn = checkInTotal,
					.checkOut = checkOutTotal,
					.cost = *reinterpret_cast<const double*>(item.at("cost")->data())
				});
			}
		}

		return result;
	}

	std::vector<CheckDetailView> GetCheckDetail(const int& _itemInventoryId, std::string_view _start, std::string_view _end)
	{
		std::vector<CheckDetailView> result;

		auto datatable = this->mysqlService.Query
			(
				"select * from checkIn where itemInventoryId = ? and time between ? and ? ",
				_itemInventoryId,
				_start,
				_end
			);
		
		for(const auto& item: datatable)
		{
			const MYSQL_TIME mysql_time = *reinterpret_cast<const MYSQL_TIME*>(item.at("time")->data());

			char temp[100]{};

			sprintf(temp, "%d-%02d-%02d %02d:%02d:%02d", mysql_time.year, mysql_time.month, mysql_time.day, mysql_time.hour, mysql_time.minute, mysql_time.second);

			result.push_back
			({
				.id = *reinterpret_cast<const int*>(item.at("id")->data()),
				.number = *reinterpret_cast<const double*>(item.at("number")->data()),
				.time = temp
			});
		}		


		 datatable = this->mysqlService.Query
			(
				"select * from checkOut where itemInventoryId = ? and time between ? and ? ",
				_itemInventoryId,
				_start,
				_end
			);
		
		for(const auto& item: datatable)
		{
			const MYSQL_TIME mysql_time = *reinterpret_cast<const MYSQL_TIME*>(item.at("time")->data());

			char temp[100]{};

			sprintf(temp, "%d-%02d-%02d %02d:%02d:%02d", mysql_time.year, mysql_time.month, mysql_time.day, mysql_time.hour, mysql_time.minute, mysql_time.second);

			result.push_back
			({
				.id = *reinterpret_cast<const int*>(item.at("id")->data()),
				.number = -*reinterpret_cast<const double*>(item.at("number")->data()),
				.time = temp
			});
		}		

		return result;
	}


	std::vector<std::string> GetCancelCheckInSql(const int& _checkInId)
	{
		auto datatable = this->mysqlService.Query("select * from checkIn where id = ?", _checkInId);		
		if(datatable.size() == 0)
			throw std::logic_error("入库记录不存在!");

		const auto itemInventory = this->itemInventoryService.GetById(*reinterpret_cast<const int*>(datatable.front().at("itemInventoryId")->data()));

		if(!itemInventory.has_value())
			throw std::logic_error("库存不存在!");


		std::stringstream ss;
		std::vector<std::string> sqlCmd;

		ss << "update itemInventory set stock = stock - " << *reinterpret_cast<const double*>(datatable.front().at("number")->data());
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "delete from checkIn where id = " << _checkInId;	
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		return sqlCmd;
	}

	std::vector<std::string> GetCancelCheckOutSql(const int& _checkOutId)
	{
		auto datatable = this->mysqlService.Query("select * from checkOut where id = ?", _checkOutId);		
		if(datatable.size() == 0)
			throw std::logic_error("出库记录不存在!");

		const auto itemInventory = this->itemInventoryService.GetById(*reinterpret_cast<const int*>(datatable.front().at("itemInventoryId")->data()));

		if(!itemInventory.has_value())
			throw std::logic_error("库存不存在!");


		std::stringstream ss;
		std::vector<std::string> sqlCmd;

		ss << "update itemInventory set stock = stock + " << *reinterpret_cast<const double*>(datatable.front().at("number")->data());
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "delete from checkOut where id = " << _checkOutId;	
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		return sqlCmd;
	}
};
