#pragma once

#include "MysqlService.hpp"

#include <vector>
#include <string>
#include <optional>
#include <sstream>

struct Material
{
	int id;
	std::string name;
};

class MaterialService
{
private:
	MysqlService mysqlService;

public:
	inline std::optional<Material> GetMaterial(std::string_view _materialName)
	{
		auto datatable = this->mysqlService.Query("select * from material where name = ?", _materialName);

		if(datatable.size() != 1)
			return {};

		return Material
		{
			.id = *reinterpret_cast<int*>(datatable.front()["id"]->data()),
			.name = datatable.front()["name"]->data()
		};
	}


	inline std::optional<Material> GetMaterial(const int& _materialId)
	{
		auto datatable = this->mysqlService.Query("select * from material where id = ?", _materialId);

		if(datatable.size() != 1)
			return {};

		return Material
		{
			.id = *reinterpret_cast<int*>(datatable.front()["id"]->data()),
			.name = datatable.front()["name"]->data()
		};
	}

	inline std::vector<std::string> GetAddMaterialSql(const int& _materialId, std::string_view _materialName)
	{
		if(_materialName.empty())
			throw std::logic_error("货物名称不能为空");

		std::stringstream sqlCmd;

		sqlCmd << "insert into material(id, name) values(" << _materialId <<  ",'" << this->mysqlService.GetSafeSqlString(_materialName) << "');";

		return {sqlCmd.str()};
	}
};
