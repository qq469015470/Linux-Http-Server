#pragma once

#include <string>
#include <vector>

class ItemInventoryService
{
private:
	MysqlService mysqlService;

public:
	std::vector<std::string> GetAddItemInventorySql(std::string_view _materialName, const int& _warehouseId, const float& _cost)
	{
		auto dataTable = this->mysqlService.Query("select * from wareHouse where id = ?", _warehouseId);

		if(dataTable.size() != 1)
			throw std::logic_error("can not find warehouse");

		std::stringstream sqlCmd;
		std::vector<std::string> result;

		dataTable = this->mysqlService.Query("select * from material where name = ?", _materialName);

		int materialId(0);
		if(dataTable.size() == 0)
		{
			materialId = this->mysqlService.GetNextInsertId("material");

			sqlCmd << "insert into material(id, name) values(" << materialId <<  ",'" << this->mysqlService.GetSafeSqlString(_materialName) << "')";

			result.push_back(sqlCmd.str());	

			sqlCmd.str("");
		}	
		else
		{
			materialId = *reinterpret_cast<int*>(dataTable[0]["id"]->data());
		}

		sqlCmd << "insert into itemInventory (wareHouseId, materialId, cost, stock) values(" << _warehouseId << "," << materialId << "," << _cost << ", 0);";
		result.push_back(sqlCmd.str());

		return result;
	}
};
