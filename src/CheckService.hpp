#pragma once 

#include "MysqlService.hpp"
#include "ItemInventoryService.hpp"

#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

struct CheckView
{
	std::string id;
	std::string name;
	double checkIn;
	double checkOut;
	double cost;
};

struct CheckDetailView
{
	std::string id;
	double number;	
	std::string time;
};

using CheckNoteView = ItemInventoryView;

class CheckService
{
private:
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

public:
	std::vector<CheckView> GetCheck(std::string_view _wareHouseId, std::string_view _start, std::string_view _end)
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
										item.at("id")->data(),
										_start,
										_end
									);
			if(checkInData.size() > 0 && checkInData.front().at("number").has_value())
			{
				checkInTotal += std::stod(checkInData.front().at("number")->data());
			}
			const auto checkOutData = this->mysqlService.Query
									(
										"select sum(number) as number from checkOut where itemInventoryId = ? and time between ? and ?",
										item.at("id")->data(),
										_start,
										_end
									);
			if(checkOutData.size() > 0 && checkOutData.front().at("number").has_value())
			{
				checkOutTotal += std::stod(checkOutData.front().at("number")->data());
			}

			if(checkInTotal != 0 || checkOutTotal != 0)
			{
				result.push_back
				({
					.id = item.at("id")->data(),
					.name = item.at("name")->data(),
					.checkIn = checkInTotal,
					.checkOut = checkOutTotal,
					.cost = std::stod(item.at("cost")->data())
				});
			}
		}

		return result;
	}

	std::vector<CheckDetailView> GetCheckDetail(std::string_view _itemInventoryId, std::string_view _start, std::string_view _end)
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
				.id = item.at("id")->data(),
				.number = std::stod(item.at("number")->data()),
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
				.id = item.at("id")->data(),
				.number = -std::stod(item.at("number")->data()),
				.time = temp
			});
		}		

		return result;
	}


	std::vector<std::string> GetCancelCheckInSql(std::string_view _checkInId)
	{
		auto datatable = this->mysqlService.Query("select * from checkIn where id = ?", _checkInId);		
		if(datatable.size() == 0)
			throw std::logic_error("入库记录不存在!");

		const auto itemInventory = this->itemInventoryService.GetById(datatable.front().at("itemInventoryId")->data());

		if(!itemInventory.has_value())
			throw std::logic_error("库存不存在!");


		std::stringstream ss;
		std::vector<std::string> sqlCmd;

		ss << "update itemInventory set stock = stock - " << std::stod(datatable.front().at("number")->data()) << " where id = '" << itemInventory->id << "' and stock >= " << std::stod(datatable.front().at("number")->data()) << ";";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "if ROW_COUNT() <> 1 then SIGNAL SQLSTATE 'HY000' SET MESSAGE_TEXT = '库存不足'; end if;";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "delete from checkIn where id = '" << _checkInId << "';";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "if ROW_COUNT() <> 1 then SIGNAL SQLSTATE 'HY000' SET MESSAGE_TEXT = '删除失败'; end if;";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		return sqlCmd;
	}

	std::vector<std::string> GetCancelCheckOutSql(std::string_view _checkOutId)
	{
		auto datatable = this->mysqlService.Query("select * from checkOut where id = ?", _checkOutId);		
		if(datatable.size() == 0)
			throw std::logic_error("出库记录不存在!");

		const auto itemInventory = this->itemInventoryService.GetById(datatable.front().at("itemInventoryId")->data());

		if(!itemInventory.has_value())
			throw std::logic_error("库存不存在!");


		std::stringstream ss;
		std::vector<std::string> sqlCmd;

		ss << "update itemInventory set stock = stock + " << std::stod(datatable.front().at("number")->data()) << " where id = '" << itemInventory->id << "';";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "if ROW_COUNT() <> 1 then SIGNAL SQLSTATE 'HY000' SET MESSAGE_TEXT = '取消出库失败'; end if;";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "delete from checkOut where id = '" << _checkOutId << "';";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		ss << "if ROW_COUNT() <> 1 then SIGNAL SQLSTATE 'HY000' SET MESSAGE_TEXT = '删除失败'; end if;";
		sqlCmd.emplace_back(ss.str());
		ss.clear();
		ss.str("");

		return sqlCmd;
	}

	std::vector<CheckNoteView> GetCheckNoteView(std::string_view _wareHouseId, std::string_view _date)
	{
		std::vector<ItemInventoryView> itemInventoryViews(this->itemInventoryService.GetByWareHouseId(_wareHouseId));

		const auto checkInDataTables = this->mysqlService.Query
		(
			"select itemInventoryId, sum(number) as number from checkIn c left join itemInventory i on c.itemInventoryId = i.id "
			"where i.wareHouseId = ? and time < DATE_ADD(STR_TO_DATE(?,'%Y-%m-%d'),INTERVAL 1 DAY) group by itemInventoryId;"
			,
			_wareHouseId,
			_date	
		);
		const auto checkOutDataTables = this->mysqlService.Query
		(
			"select itemInventoryId, sum(number) as number from checkOut c left join itemInventory i on c.itemInventoryId = i.id "
			"where i.wareHouseId = ? and time < DATE_ADD(STR_TO_DATE(?,'%Y-%m-%d'),INTERVAL 1 DAY) group by itemInventoryId;"
			,
			_wareHouseId,
			_date	
		);

		for(auto& item: itemInventoryViews)
		{
			item.stock = 0;

			auto checkInIter = std::find_if(checkInDataTables.begin(), checkInDataTables.end(), [&item](const std::unordered_map<std::string, std::optional<std::vector<char>>>& elem) 
			{
				return std::string_view(elem.at("itemInventoryId").value().data()) == item.id;
			});

			if(checkInIter != checkInDataTables.end())
			{
				item.stock += std::stod(checkInIter->at("number")->data());
			}

			auto checkOutIter = std::find_if(checkOutDataTables.begin(), checkOutDataTables.end(), [&item](const std::unordered_map<std::string, std::optional<std::vector<char>>>& elem) 
			{
				return std::string_view(elem.at("itemInventoryId").value().data()) == item.id;
			});

			if(checkOutIter != checkOutDataTables.end())
			{
				item.stock -= std::stod(checkOutIter->at("number")->data());
			}
		}

		//"select itemInventoryId, sum(number) as number from checkIn c left join itemInventory i on c.itemInventoryId = i.id  where i.wareHouseId = 'b39f64c4-ab37-11ec-acff-000c29910818' and time < DATE_ADD(STR_TO_DATE('2000-01-02','%Y-%m-%d'),INTERVAL 1 DAY) group by itemInventoryId;";

		return itemInventoryViews;
	}
};
