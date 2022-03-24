#pragma once

#include "MaterialService.hpp"

#include <string>
#include <vector>
#include <sstream>

struct ItemInventoryView
{
	std::string id;
	std::string wareHouseId;
	std::string materialId;
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
	inline std::vector<std::string> GetCheckInSql(std::string_view _itemInventoryId, const double& _stock, bool _checkVaild = true)
	{
		if(_stock <= 0)
			throw std::logic_error("入库数量必须大于0");

		auto datatable = this->mysqlService.Query("select * from itemInventory where id = ?", _itemInventoryId); 
		if(_checkVaild && datatable.size() != 1)
			throw std::logic_error("找不到库存");
			

		std::vector<std::string> result;
		std::stringstream sqlCmd;

		sqlCmd << "insert into checkIn(id, itemInventoryId, number, time)values(uuid(),'" << this->mysqlService.GetSafeSqlString(_itemInventoryId) << "'," << _stock << ",now());";
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		sqlCmd << "update itemInventory set stock = stock + " << _stock << " where id = '" << this->mysqlService.GetSafeSqlString(_itemInventoryId) << "';"; 
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		return result;		
	}

	inline std::vector<std::string> GetCheckOutSql(std::string_view _itemInventoryId, const double& _stock)
	{
		if(_stock <= 0)
			throw std::logic_error("出库数量必须大于0");


		auto datatable = this->mysqlService.Query("select * from itemInventory where id = ?", _itemInventoryId); 
		if(datatable.size() != 1)
			throw std::logic_error("找不到库存");

		if(std::stod(datatable[0]["stock"]->data()) - _stock < 0)
			throw std::logic_error("库存不足");

		std::vector<std::string> result;
		std::stringstream sqlCmd;

		sqlCmd << "insert into checkOut(id, itemInventoryId, number, time)values(uuid(),'" << this->mysqlService.GetSafeSqlString(_itemInventoryId) << "'," << _stock << ",now());";
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		sqlCmd << "update itemInventory set stock = stock - " << _stock << " where id = '" << this->mysqlService.GetSafeSqlString(_itemInventoryId) << "' and stock >= " << _stock << ";"; 
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		return result;		
	}

	inline std::vector<std::string> GetAddItemInventorySql(std::string_view _id, std::string_view _materialName, std::string_view _wareHouseId, const double& _cost)
	{
		auto dataTable = this->mysqlService.Query("select * from wareHouse where id = ?", _wareHouseId);

		if(dataTable.size() != 1)
			throw std::logic_error("仓库不存在!");

		std::stringstream sqlCmd;
		std::vector<std::string> result;

		const std::optional<Material> material = this->materialService.GetMaterialByName(_materialName);

		std::string materialId;
		if(!material.has_value())
		{
			materialId = this->mysqlService.GetUUID();

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

		sqlCmd << "insert into itemInventory (id, wareHouseId, materialId, cost, stock) values('" << this->mysqlService.GetSafeSqlString(_id) << "','" << this->mysqlService.GetSafeSqlString(_wareHouseId) << "','" << this->mysqlService.GetSafeSqlString(materialId) << "'," << _cost << ", 0);";
		result.emplace_back(sqlCmd.str());

		return result;
	}

	inline std::vector<std::string> GetEditItemInventorySql(std::string_view _itemInventoryId, std::string_view _name, const double& _price)
	{
		auto dataTable = this->mysqlService.Query("select * from itemInventory where id = ?", _itemInventoryId);
		if(dataTable.size() != 1)
			throw std::logic_error("库存不存在!");

		std::optional<Material> material = this->materialService.GetMaterialById(dataTable.front()["materialId"]->data());
		if(!material.has_value())
			throw std::logic_error("物料不存在!");
		
		std::stringstream sqlCmd;
		std::vector<std::string> result;

		sqlCmd << "update itemInventory set cost = " << _price << " where id = '" << this->mysqlService.GetSafeSqlString(_itemInventoryId) << "';";
		result.emplace_back(sqlCmd.str());
		sqlCmd.str("");

		for(const auto& item: this->materialService.GetEditMaterialSql(material->id, _name))
		{
			result.emplace_back(item);
		}

		return result;
	}

	inline std::vector<ItemInventoryView> GetByWareHouseId(std::string_view _wareHouseId)
	{
		auto dataTable = this->mysqlService.Query("select itemInventory.*, material.name from itemInventory left join material on itemInventory.materialId = material.id where itemInventory.wareHouseId = ?", _wareHouseId);

		std::vector<ItemInventoryView> result;			
		for(const auto& item: dataTable)
		{
			ItemInventoryView temp
			{
				.id =  item.at("id")->data(),
				.wareHouseId =  item.at("wareHouseId")->data(),
				.materialId =  item.at("materialId")->data(),
				.name = item.at("name")->data(),
				.price = std::stod(item.at("cost")->data()),
				.stock = std::stod(item.at("stock")->data()),
			};

			result.emplace_back(temp);
		}

		return result;
	}

	inline std::vector<Material> GetContainsMaterialName(std::string_view _wareHouseId, std::string_view _materialName)
	{
		auto dataTable = this->mysqlService.Query
						(
						 "select material.id, material.name from itemInventory "
						 "left join material on itemInventory.materialId = material.id "
						 "where wareHouseId = ? and material.name like CONCAT('%',?,'%')",
						 _wareHouseId,
						 _materialName
						);

		std::vector<Material> result;
		for(const auto& item: dataTable)
		{
			result.push_back
			({
				.id = item.at("id")->data(),
				.name = item.at("name")->data(),
			});	
		}

		return result;
	}

	inline std::optional<ItemInventoryView> GetById(std::string_view _itemInventoryId)
	{
		auto dataTable = this->mysqlService.Query("select itemInventory.*, material.name from itemInventory left join material on itemInventory.materialId = material.id where itemInventory.id = ?", _itemInventoryId);

		std::optional<ItemInventoryView> result;			
		if(dataTable.size() > 0)
		{
			result = 
			{
				.id = dataTable.front().at("id")->data(),
				.wareHouseId = dataTable.front().at("wareHouseId")->data(),
				.materialId = dataTable.front().at("materialId")->data(),
				.name = dataTable.front().at("name")->data(),
				.price = std::stod(dataTable.front().at("cost")->data()),
				.stock = std::stod(dataTable.front().at("stock")->data()),
			};
		}

		return result;
	}
};
