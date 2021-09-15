#pragma once

#include "MysqlSubmitService.hpp"

#include <string>

struct WareHouse
{
	int32_t id;
	std::string name;
};

class WareHouseService
{
private:
	MysqlSubmitService mysqlSubmitService; 

public:
	std::vector<WareHouse> GetWareHouses()
	{
		auto dataTable = this->mysqlSubmitService.Query("select * from WareHouse");

		std::vector<WareHouse> result;
		for(const auto& item: dataTable)
		{
			result.push_back({*reinterpret_cast<const int32_t*>(item.at("id").value().data()), item.at("name").value().data()});	
		}

		return result;	
	}

	void AddWareHouse(std::string_view _name)
	{
		auto dataTable = this->mysqlSubmitService.Query("select 1 from WareHouse where name = ?", _name);
		if(dataTable.size() > 0)
			throw std::logic_error("仓库名称不能重复!");

		this->mysqlSubmitService.ExecuteCommand("insert into WareHouse(name) values(?)", _name);
	}
};
