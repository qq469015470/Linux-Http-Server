#pragma once

#include <mysql/mysql.h>

#include <string>
#include <stdexcept>
#include <unordered_map> 
#include <optional>
#include <list>

class MysqlService
{
private:
	static inline std::string host = "";
	static inline std::string username = "";
	static inline std::string password = "";
	static inline std::string databaseName = "";

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

	void ExecuteCommandPrepareStatement_ConvertBind(const double& _number, std::vector<MYSQL_BIND>& _sql_binds, std::list<unsigned long>& _length)
	{
		MYSQL_BIND temp;

		temp.buffer_type = MYSQL_TYPE_DOUBLE;
		temp.buffer = const_cast<double*>(&_number);

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

	inline std::vector<std::unordered_map<std::string, std::optional<std::vector<char>>>> GetQueryResult(MYSQL_STMT* _stmt)
	{
		//MYSQL_RES* result(mysql_store_result(&mysql));
		MYSQL_RES* result(mysql_stmt_result_metadata(_stmt));
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

		std::vector<unsigned long> real_length;
		real_length.resize(binds.size());

		bool errors[256];
		bool isNull[256];

		memset(errors, 0, sizeof(errors));
		memset(isNull, 0, sizeof(isNull));

		//读取表字段
		for(int i = 0; i < num_fields; i++)
		{
			//std::cout << "field" << i << " name is " << fields[i].name << std::endl;
			//std::cout << fields[i].type << "|MYSQL_TYPE_VAR_STRING:" << MYSQL_TYPE_VAR_STRING << std::endl;

			fieldMap.insert(std::pair(i, fields[i].name));


			binds.at(i) = {};

			switch(fields[i].type)
			{
				case MYSQL_TYPE_STRING:
				case MYSQL_TYPE_VAR_STRING:
				case MYSQL_TYPE_NEWDECIMAL:
					buffers.at(i).resize(fields[i].length);
					binds.at(i).buffer_length = buffers.at(i).size();
					break;
				case MYSQL_TYPE_LONG:
					buffers.at(i).resize(8);
					break;
				case MYSQL_TYPE_DOUBLE:
					buffers.at(i).resize(8);
					break;
				case MYSQL_TYPE_DATE:
				case MYSQL_TYPE_DATETIME:
					buffers.at(i).resize(sizeof(MYSQL_TIME));
					break;
				case MYSQL_TYPE_LONGLONG:
					buffers.at(i).resize(fields[i].length);
					break;
				default:
					throw std::runtime_error("not implement");	
					break;
			}


			//memset(&binds.at(i), 0, sizeof(MYSQL_BIND));
			binds.at(i).buffer_type = fields[i].type;
			binds.at(i).buffer = buffers.at(i).data();

			binds.at(i).length = &real_length.at(i); 
			binds.at(i).error = &errors[i];
			binds.at(i).is_null = &isNull[i];
		}


		if(mysql_stmt_bind_result(this->stmt, binds.data()))
			throw std::runtime_error(mysql_stmt_error(this->stmt));

		if (mysql_stmt_store_result(this->stmt))
			throw std::runtime_error(mysql_stmt_error(this->stmt));


		std::vector<std::unordered_map<std::string, std::optional<std::vector<char>>>> dataTable;

		while(!mysql_stmt_fetch(this->stmt))
		{
			std::unordered_map<std::string, std::optional<std::vector<char>>> temp;

			for(int i = 0; i < num_fields; i++)
			{
				std::optional<std::vector<char>> value;

				if(!*binds.at(i).is_null)
				{
					value = buffers[i];
				}

				const std::pair<decltype(temp)::iterator, bool> insertRes = temp.insert(std::pair(fieldMap.at(i), std::move(value)));

				assert(insertRes.second == true);
			}	

			dataTable.emplace_back(std::move(temp));
		}

		mysql_free_result(result);

		return dataTable;
	}

public:
	MysqlService()
	{
		mysql_init(&this->mysql);
		mysql_real_connect(&this->mysql, std::remove_pointer<decltype(this)>::type::host.c_str(), std::remove_pointer<decltype(this)>::type::username.c_str(), std::remove_pointer<decltype(this)>::type::password.c_str(), std::remove_pointer<decltype(this)>::type::databaseName.c_str(), 3306, NULL, 0);

		this->stmt = mysql_stmt_init(&this->mysql);
		if(!this->stmt)
		{
			throw std::runtime_error("mysql_stmt_init(), out of memory");
		}
	}

	~MysqlService()
	{
		mysql_stmt_close(this->stmt);
		mysql_close(&this->mysql);
	}

	static inline void SetConnect(std::string _host, std::string _username, std::string _password, std::string _databaseName)
	{
		host = _host;
		username = _username;
		password = _password;
		databaseName = _databaseName;
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
		if(mysql_real_query(&this->mysql, _cmdStr.data(), _cmdStr.size()))
		{
			const std::string errStr = mysql_error(&this->mysql);
			
			if(errStr.empty())
				throw std::logic_error("MYSQL query is error");
			else
				throw std::logic_error(errStr);
		}

		return mysql_insert_id(&this->mysql);
	}

	inline void ExecuteCommandWithTran(const std::vector<std::string>& _cmdStrs)
	{
		const std::string threadId = std::to_string(std::hash<std::thread::id>()(std::this_thread::get_id()));
		const std::string procedureName = "TEST_" + threadId;
		this->ExecuteCommand("DROP PROCEDURE IF EXISTS " + procedureName);
		std::string baseSql
		(
			
			std::string("CREATE PROCEDURE ") + procedureName + "()\r\n"
		);


		baseSql += 
			"BEGIN\r\n"
			"START TRANSACTION;\r\n";

		for(const auto& item: _cmdStrs)
		{
			baseSql += item;
			baseSql += "\r\n";
		}

		baseSql += "COMMIT;\r\n";
		baseSql += "END\r\n";

		this->ExecuteCommand(baseSql);
		try
		{
			this->ExecuteCommand(std::string("CALL ") + procedureName + "()");
		}
		catch(std::exception& _ex)
		{
			this->ExecuteCommand(std::string("DROP PROCEDURE ") + procedureName);
			throw;
		}
		this->ExecuteCommand(std::string("DROP PROCEDURE ") + procedureName);
	}

	template<typename... ARGS>	
	inline std::vector<std::unordered_map<std::string, std::optional<std::vector<char>>>> Query(std::string_view _cmdStr, const ARGS& ...args)
	{
		this->ExecuteCommand(_cmdStr, args...);

		return GetQueryResult(this->stmt);
	}

	inline std::vector<std::unordered_map<std::string, std::optional<std::vector<char>>>> Query(std::string_view _cmdStr)
	{
		this->ExecuteCommand_WithParam(_cmdStr, {});

		return GetQueryResult(this->stmt);
	}


	inline long GetNextInsertId(std::string_view _table)
	{
		this->ExecuteCommand(std::string("ANALYZE TABLE ") + _table.data());

		MYSQL_RES* res(mysql_use_result(&this->mysql));
		mysql_free_result(res);

		auto datatable = this->Query("SELECT AUTO_INCREMENT FROM information_schema.`TABLES` WHERE TABLE_SCHEMA = database() AND TABLE_NAME = ?", _table.data());

		return *reinterpret_cast<long*>(datatable[0]["AUTO_INCREMENT"].value().data());
	}

	inline std::string GetUUID()
	{
		auto datatable = this->Query("SELECT uuid()");

		return datatable[0]["uuid()"].value().data();
	}
	
	inline std::string GetSafeSqlString(std::string_view _sqlStr)
	{
		std::string result;

		result.resize(_sqlStr.size() * 2 + 1);

		int end = mysql_real_escape_string(&this->mysql, result.data(), _sqlStr.data(), _sqlStr.size());

		result.resize(end);

		return result;
	}
};
