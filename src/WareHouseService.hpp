#pragma once

#include "MysqlSubmitService.hpp"

#include <string>

struct WareHouse
{
	uint64_t id;
	std::string name;
};

class WareHouseService
{
private:
	MysqlSubmitService mysqlSubmitService; 

public:
	std::vector<WareHouse> GetWareHouses()
	{
		return {{123, "test"}};	
	}
};
