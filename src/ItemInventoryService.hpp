#pragma once

#include "MaterialService.hpp"

#include <string>
#include <vector>
#include <sstream>

struct ItemInventoryView
{
	int id;
	int wareHouseId;
	int materialId;
	std::string name;
	double price;
	double stock;
};

class ItemInventoryService
{
private:
	MysqlService mysqlService;
	MaterialService materialService;

public:
	inline std::vector<std::string> GetCheckInSql(const int& _itemInventoryId, const double& _stock, bool _checkVaild = true)
	{
		if(_stock <= 0)
			throw std::logic_error("入库数量必须大于0");

		auto datatable = this->mysqlService.Query("select * from itemInventory where id = ?", _itemInventoryId); 
		if(_checkVaild && datatable.size() != 1)
			throw std::logic_error("找不到库存");
			

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

	inline std::vector<std::string> GetCheckOutSql(const int& _itemInventoryId, const double& _stock)
	{
		if(_stock <= 0)
			throw std::logic_error("出库数量必须大于0");


		auto datatable = this->mysqlService.Query("select * from itemInventory where id = ?", _itemInventoryId); 
		if(datatable.size() != 1)
			throw std::logic_error("找不到库存");

		if(*reinterpret_cast<double*>(datatable[0]["stock"]->data()) - _stock < 0)
			throw std::logic_error("库存不足");

		std::vector<std::string> result;
		std::stringstream sqlCmd;

		sqlCmd << "insert into checkOut(itemInventoryId, number, time)values(" << _itemInventoryId << "," << _stock << ",now())";
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		sqlCmd << "update itemInventory set stock = stock - " << _stock << " where id = " << _itemInventoryId; 
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		return result;		
	}

	inline std::vector<std::string> GetAddItemInventorySql(std::string_view _materialName, const int& _wareHouseId, const double& _cost)
	{
		std::cout << "materialName:" << _materialName << std::endl;

		auto dataTable = this->mysqlService.Query("select * from wareHouse where id = ?", _wareHouseId);

		if(dataTable.size() != 1)
			throw std::logic_error("仓库不存在!");

		std::stringstream sqlCmd;
		std::vector<std::string> result;

		const std::optional<Material> material = this->materialService.GetMaterial(_materialName);

		int materialId(0);
		if(!material.has_value())
		{
			materialId = this->mysqlService.GetNextInsertId("material");

			for(const auto& item: this->materialService.GetAddMaterialSql(materialId, _materialName))
			{
				result.emplace_back(item);	
			}
		}	
		else
		{
			materialId = material->id;
		}

		dataTable = this->mysqlService.Query("select * from itemInventory where materialId = ? and warehouseId = ? and cost = ?", materialId, _wareHouseId, _cost);
		if(dataTable.size() != 0)
			throw std::logic_error("不能重复添加!");

		sqlCmd << "insert into itemInventory (wareHouseId, materialId, cost, stock) values(" << _wareHouseId << "," << materialId << "," << _cost << ", 0);";
		result.emplace_back(sqlCmd.str());

		return result;
	}

	inline std::vector<ItemInventoryView> GetByWareHouseId(const int& _wareHouseId)
	{
		auto dataTable = this->mysqlService.Query("select itemInventory.*, material.name from itemInventory left join material on itemInventory.materialId = material.id where itemInventory.wareHouseId = ?", _wareHouseId);				

		std::vector<ItemInventoryView> result;			
		for(const auto& item: dataTable)
		{
			ItemInventoryView temp
			{
				.id =  *reinterpret_cast<const int*>(item.at("id")->data()),
				.wareHouseId =  *reinterpret_cast<const int*>(item.at("wareHouseId")->data()),
				.materialId =  *reinterpret_cast<const int*>(item.at("materialId")->data()),
				.name = item.at("name")->data(),
				.price = *reinterpret_cast<const double*>(item.at("cost")->data()),
				.stock = *reinterpret_cast<const double*>(item.at("stock")->data()),
			};

			result.emplace_back(temp);
		}

		return result;
	}
};
