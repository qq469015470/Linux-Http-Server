#pragma once 

#include "MysqlService.hpp"

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

class CheckService
{
private:
	MysqlService mysqlService;

public:
	std::vector<CheckView> GetCheck(const int& _wareHouseId, std::string_view _start, std::string_view _end)
	{
		std::vector<CheckView> result;

		auto datatable = this->mysqlService.Query
			(
				"select checkIn.time, material.name, itemInventory.cost from checkIn "
				"left join itemInventory on checkIn.itemInventoryId = itemInventory.id "
			        "left join material on itemInventory.materialId = material.id "
				"where itemInventory.warehouseId = ? and time between ? and ?", 
				_wareHouseId, 
				_start, 
				_end
			);
		for(const auto& item: datatable)
		{
			result.push_back
			({
				.id = *reinterpret_cast<const int*>(item.at("id")->data()),
				.name = item.at("name")->data(),
				.checkIn = *reinterpret_cast<const double*>(item.at("checkIn")->data()),
				.cost = *reinterpret_cast<const double*>(item.at("cost")->data())
			});
		}	

		return result;
	}
};
