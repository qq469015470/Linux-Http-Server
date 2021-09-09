#include <mysql/mysql.h>

#include <string>
#include <stdexcept>
#include <unordered_map> 
#include <optional>

class MysqlSubmitService
{
private:
	MYSQL mysql;

public:
	MysqlSubmitService()
	{
		mysql_init(&this->mysql);
		mysql_real_connect(&this->mysql, "localhost", "root", "123456", "cpptest", 3306, NULL, 0);
	}

	~MysqlSubmitService()
	{
		mysql_close(&this->mysql);
	}
	

	inline int ExecuteCommand(std::string_view _cmdStr)
	{
		if(mysql_real_query(&this->mysql, _cmdStr.data(), _cmdStr.size()))
		{
			const std::string errStr = mysql_error(&mysql);
			
			if(errStr.empty())
				throw std::runtime_error("MYSQL query is error");
			else
				throw std::runtime_error(errStr);
		}

		return mysql_insert_id(&this->mysql);
	}

	inline std::vector<std::unordered_map<std::string, std::optional<std::string>>> Query(std::string_view _cmdStr)
	{
		this->ExecuteCommand(_cmdStr);

		try
		{
			MYSQL_RES* result(mysql_store_result(&mysql));
			if(!result)
				throw std::runtime_error("MYSQL not result!");	

			const int num_fields = mysql_num_fields(result);
			if(num_fields == 0)
				throw std::runtime_error("MYSQL fields number is 0!");

			const MYSQL_FIELD* fields = mysql_fetch_fields(result);
			if(!fields)
				throw std::runtime_error("MYSQL fields fetch is error!");

			std::unordered_map<int, std::string> fieldMap;
			//读取表字段
			for(int i = 0; i < num_fields; i++)
			{
				//std::cout << "field" << i << " name is " << fields[i].name << std::endl;
				fieldMap.insert(std::pair(i, fields[i].name));
			}

			std::vector<std::unordered_map<std::string, std::optional<std::string>>> dataTable;

			//读取值
			while(MYSQL_ROW row = mysql_fetch_row(result))
			{
				std::unordered_map<std::string, std::optional<std::string>> temp;

				for(int i = 0; i < num_fields; i++)
				{
					std::optional<std::string> value;
					if(row[i] != nullptr)
						value = row[i];

					const std::pair<decltype(temp)::iterator, bool> insertRes = temp.insert(std::pair(fieldMap[i], std::move(value)));

					assert(insertRes.second == true);

					//if(NULL == row[i])
					//{
					//	std::cout << " null";
					//}
					//else
					//{
					//	std::cout << " " << row[i];
					//}
				}
				//std::cout << std::endl;

				dataTable.emplace_back(std::move(temp));
			}

			//std::cout << "MYSQL is OK." << std::endl;

			return dataTable;
		}
		catch (std::runtime_error& error)
		{
			std::cout << error.what() << std::endl;	
		}
		catch (...)
		{
			std::cout << "MYSQL operation is error!" << std::endl;
		}
	}


	uint64_t GetNextInsertId(std::string_view _table)
	{
		auto datatable = this->Query("SELECT AUTO_INCREMENT FROM information_schema.`TABLES` WHERE TABLE_SCHEMA = database() AND TABLE_NAME = 'user'");

		return std::stoll(datatable[0]["AUTO_INCREMENT"].value());
	}
};
