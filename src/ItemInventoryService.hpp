#pragma once

#include <string>
#include <vector>
#include <sstream>

class ItemInventoryService
{
private:
	MysqlService mysqlService;

public:
	std::vector<std::string> GetCheckInSql(const int& _itemInventoryId, const double& _stock)
	{
		std::vector<std::string> result;
		std::stringstream sqlCmd;

		sqlCmd << "insert into checkIn(itemInventoryId, number, time)values(" << _itemInventoryId << "," << _stock << ",now())";
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		sqlCmd << "update itemInventory set stock = stock + " << _stock << " where id = " << _itemInventoryId; 
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		return result;		
	}

	std::vector<std::string> GetAddItemInventorySql(std::string_view _materialName, const int& _warehouseId, const double& _cost)
	{
		std::cout << "materialName:" << _materialName << std::endl;

		auto dataTable = this->mysqlService.Query("select * from wareHouse where id = ?", _warehouseId);

		if(dataTable.size() != 1)
			throw std::logic_error("仓库不存在!");

		std::stringstream sqlCmd;
		std::vector<std::string> result;

		dataTable = this->mysqlService.Query("select * from material where name = ?", _materialName);

		int materialId(0);
		if(dataTable.size() == 0)
		{
			materialId = this->mysqlService.GetNextInsertId("material");

			sqlCmd << "insert into material(id, name) values(" << materialId <<  ",'" << this->mysqlService.GetSafeSqlString(_materialName) << "')";

			result.emplace_back(sqlCmd.str());	

			sqlCmd.clear();
			sqlCmd.str("");
		}	
		else
		{
			materialId = *reinterpret_cast<int*>(dataTable[0]["id"]->data());
		}

		dataTable = this->mysqlService.Query("select * from itemInventory where materialId = ? and warehouseId = ? and cost = ?", materialId, _warehouseId, _cost);
		if(dataTable.size() != 0)
			throw std::logic_error("不能重复添加!");

		sqlCmd << "insert into itemInventory (wareHouseId, materialId, cost, stock) values(" << _warehouseId << "," << materialId << "," << _cost << ", 0);";
		result.emplace_back(sqlCmd.str());

		return result;
	}
};
