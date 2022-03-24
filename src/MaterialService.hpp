#pragma once

#include "MysqlService.hpp"

#include <vector>
#include <string>
#include <optional>
#include <sstream>

struct Material
{
	std::string id;
	std::string name;
};

class MaterialService
{
private:
	MysqlService mysqlService;

	inline void CheckNameVaild(std::string_view _materialName)
	{
		if(_materialName.empty())
			throw std::logic_error("货物名称不能为空");

		if(this->GetMaterialByName(_materialName).has_value())
			throw std::logic_error("货物名称不能重复");
	}

public:
	inline std::optional<Material> GetMaterialByName(std::string_view _materialName)
	{
		auto datatable = this->mysqlService.Query("select * from material where name = ?", _materialName);

		if(datatable.size() != 1)
			return {};

		return Material
		{
			.id = datatable.front()["id"]->data(),
			.name = datatable.front()["name"]->data()
		};
	}


	inline std::optional<Material> GetMaterialById(std::string_view _materialId)
	{
		auto datatable = this->mysqlService.Query("select * from material where id = ?", _materialId);

		if(datatable.size() != 1)
			return {};

		return Material
		{
			.id = datatable.front()["id"]->data(),
			.name = datatable.front()["name"]->data()
		};
	}

	inline std::vector<std::string> GetAddMaterialSql(std::string_view _materialId, std::string_view _materialName)
	{
		this->CheckNameVaild(_materialName);

		std::stringstream sqlCmd;

		sqlCmd << "insert into material(id, name) values('" << this->mysqlService.GetSafeSqlString(_materialId) <<  "','" << this->mysqlService.GetSafeSqlString(_materialName) << "');";

		return {sqlCmd.str()};
	}

	inline std::vector<std::string> GetEditMaterialSql(std::string_view _materialId, std::string_view _materialName)
	{
		this->CheckNameVaild(_materialName);

		if(!this->GetMaterialById(_materialId).has_value())
			throw std::logic_error("货物不存在");
		
		std::stringstream sqlCmd;

		sqlCmd << "update material set name = '" << this->mysqlService.GetSafeSqlString(_materialName) << "' where id = '" << this->mysqlService.GetSafeSqlString(_materialId) << "';";
		sqlCmd << "if ROW_COUNT() <> 1 then SIGNAL SQLSTATE 'HY000' SET MESSAGE_TEXT = '修改货物名称失败'; end if;";

		return {sqlCmd.str()};
	}
};
