#pragma once

#include <mysql/mysql.h>

#include <string>
#include <stdexcept>
#include <unordered_map> 
#include <optional>
#include <list>

class MysqlSubmitService
{
private:
	MYSQL mysql;
	MYSQL_STMT* stmt;

	void ExecuteCommandPrepareStatement_ConvertBind(std::string_view _str, std::vector<MYSQL_BIND>& _sql_binds, std::list<unsigned long>& _length)
	{
		MYSQL_BIND temp;

		temp.buffer_type = MYSQL_TYPE_STRING;
		temp.buffer = const_cast<char*>(_str.data());

		_length.push_back(_str.size());
		temp.length = &_length.back();

		temp.is_null = 0;
		
		_sql_binds.emplace_back(temp);
	}

	void ExecuteCommandPrepareStatement_ConvertBind(const int& _number, std::vector<MYSQL_BIND>& _sql_binds, std::list<unsigned long>& _length)
	{
		MYSQL_BIND temp;

		temp.buffer_type = MYSQL_TYPE_LONG;
		temp.buffer = const_cast<int*>(&_number);

		_length.push_back(sizeof(_number));
		temp.length = &_length.back();

		temp.is_null = 0;
		
		_sql_binds.emplace_back(temp);
	}


	template<typename T, typename... ARGS>
	inline int ExecuteCommandPrepareStatement(std::string_view _cmdStr, std::vector<MYSQL_BIND>& _sql_binds, std::list<unsigned long>& _length, const T& _t, const ARGS& ...args)
	{
		this->ExecuteCommandPrepareStatement_ConvertBind(_t, _sql_binds, _length);
	
		return this->ExecuteCommandPrepareStatement(_cmdStr, _sql_binds, _length, args...);
	}


	template<typename T>
	inline int ExecuteCommandPrepareStatement(std::string_view _cmdStr, std::vector<MYSQL_BIND>& _sql_binds, std::list<unsigned long>& _length, const T& _t)
	{
		this->ExecuteCommandPrepareStatement_ConvertBind(_t, _sql_binds, _length);

		this->ExecuteCommand_WithParam(_cmdStr, _sql_binds);

		//std::cout << "last call" << std::endl;
		return -1;
	}


	inline int ExecuteCommand_WithParam(std::string_view _cmdStr, std::vector<MYSQL_BIND> _binds) 
	{
		if(mysql_stmt_prepare(this->stmt, _cmdStr.data(), _cmdStr.size()))
			throw std::runtime_error(mysql_stmt_error(stmt));

		if(_binds.size() > 0)
		{
			if(mysql_stmt_bind_param(this->stmt, _binds.data()))
				throw std::runtime_error(mysql_stmt_error(this->stmt));
		}

		//if(mysql_real_query(&this->mysql, _cmdStr.data(), _cmdStr.size()))
		if(mysql_stmt_execute(this->stmt))
		{
			//const std::string errStr = mysql_error(&mysql);
			const std::string errStr = mysql_stmt_error(this->stmt);
			
			if(errStr.empty())
				throw std::runtime_error("MYSQL query is error");
			else
				throw std::runtime_error(errStr);
		}

		return mysql_insert_id(&this->mysql);
	}

public:
	MysqlSubmitService()
	{
		mysql_init(&this->mysql);
		mysql_real_connect(&this->mysql, "localhost", "root", "123456", "dangkou", 3306, NULL, 0);

		this->stmt = mysql_stmt_init(&this->mysql);
		if(!this->stmt)
		{
			throw std::runtime_error("mysql_stmt_init(), out of memory");
		}
	}

	~MysqlSubmitService()
	{
		mysql_stmt_close(this->stmt);
		mysql_close(&this->mysql);
	}

	template<typename... ARGS>
	inline int ExecuteCommand(std::string_view _cmdStr, const ARGS& ...args)
	{
		std::vector<MYSQL_BIND> binds;
		std::list<unsigned long> length;

		return this->ExecuteCommandPrepareStatement(_cmdStr, binds, length, args...); 
	}
	
	inline int ExecuteCommand(std::string_view _cmdStr) 
	{
		return this->ExecuteCommand_WithParam(_cmdStr, {});
	}

	template<typename... ARGS>	
	inline std::vector<std::unordered_map<std::string, std::optional<std::vector<char>>>> Query(std::string_view _cmdStr, const ARGS& ...args)
	{
		this->ExecuteCommand(_cmdStr, args...);

		//MYSQL_RES* result(mysql_store_result(&mysql));
		MYSQL_RES* result(mysql_stmt_result_metadata(this->stmt));
		if(!result)
			throw std::runtime_error("MYSQL not result!");	

		const int num_fields = mysql_num_fields(result);
		if(num_fields == 0)
			throw std::runtime_error("MYSQL fields number is 0!");

		const MYSQL_FIELD* fields = mysql_fetch_fields(result);
		if(!fields)
			throw std::runtime_error("MYSQL fields fetch is error!");

		std::unordered_map<int, std::string> fieldMap;
		std::vector<MYSQL_BIND> binds;
		binds.resize(num_fields);
		std::vector<std::vector<char>> buffers;
		buffers.resize(binds.size());
		//读取表字段
		for(int i = 0; i < num_fields; i++)
		{
			//std::cout << "field" << i << " name is " << fields[i].name << std::endl;
			//std::cout << fields[i].type << "|MYSQL_TYPE_VAR_STRING:" << MYSQL_TYPE_VAR_STRING << std::endl;

			fieldMap.insert(std::pair(i, fields[i].name));

			buffers[i].resize(fields[i].length);


			binds[i].buffer_type = fields[i].type;
			binds[i].buffer = buffers[i].data();
			binds[i].buffer_length = buffers[i].size();
		}


		//unsigned long length[2];
		//bool          error[4];
		//bool          is_null[4];

		//int int_data;
		/* INTEGER COLUMN */
		//binds[0].buffer_type= MYSQL_TYPE_LONG;
		//binds[0].buffer= buffers[0].data(); //(char *)&int_data;
		//binds[0].is_null= &is_null[0];
		//binds[0].length= &length[0];
		//binds[0].error= &error[0];
		
		//std::string str_data;
		//str_data.resize(255);
		/* STRING COLUMN */
		//binds[1].buffer_type= MYSQL_TYPE_STRING;
		//binds[1].buffer= buffers[1].data(); //(char *)str_data.data();
		//binds[1].buffer_length= buffers[1].size(); //str_data.size();
		//binds[1].is_null= &is_null[1];
		//binds[1].length= &length[1];
		//binds[1].error= &error[1];


		std::vector<std::unordered_map<std::string, std::optional<std::vector<char>>>> dataTable;

		if(mysql_stmt_bind_result(this->stmt, binds.data()))
			throw std::runtime_error(mysql_stmt_error(stmt));

		if (mysql_stmt_store_result(this->stmt))
			throw std::runtime_error(mysql_stmt_error(stmt));

		while(!mysql_stmt_fetch(this->stmt))
		{
			std::unordered_map<std::string, std::optional<std::vector<char>>> temp;

			for(int i = 0; i < num_fields; i++)
			{
				std::optional<std::vector<char>> value;
				value = buffers[i];

				const std::pair<decltype(temp)::iterator, bool> insertRes = temp.insert(std::pair(fieldMap.at(i), std::move(value)));

				assert(insertRes.second == true);
			}	

			dataTable.emplace_back(std::move(temp));
		}

		mysql_free_result(result);

		return dataTable;
	}

	long GetNextInsertId(std::string_view _table)
	{
		auto datatable = this->Query("SELECT AUTO_INCREMENT FROM information_schema.`TABLES` WHERE TABLE_SCHEMA = database() AND TABLE_NAME = 'user'");

		return *reinterpret_cast<long*>(datatable[0]["AUTO_INCREMENT"].value().data());
	}
};
