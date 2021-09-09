#include <gtest/gtest.h>
#include "../src/MysqlSubmitService.hpp"

TEST(MysqlSubmitService, DropTable)
{
	MysqlSubmitService service;

	const std::string sqlCmd = "DROP TABLE IF EXISTS `user`";

	service.ExecuteCommand(sqlCmd);
}

TEST(MysqlSubmitService, CreateTable)
{
	MysqlSubmitService service;

	const std::string sqlCmd = 
		"CREATE TABLE IF NOT EXISTS `user` ("
  			"`id` int NOT NULL AUTO_INCREMENT,"
  			"`name` varchar(20) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,"
  			"PRIMARY KEY (`id`)"
  		");";

	service.ExecuteCommand(sqlCmd);
}


TEST(MysqlSubmitService, Insert)
{
	{
		MysqlSubmitService service;

		const std::string sqlCmd = "INSERT INTO user(id, name) VALUES(2, '测试')"; 

		uint64_t id = service.ExecuteCommand(sqlCmd);

		EXPECT_EQ(2, id);
	}

	{
		MysqlSubmitService service;

		const std::string sqlCmd = "INSERT INTO user(name) VALUES(NULL)"; 

		uint64_t id = service.ExecuteCommand(sqlCmd);
		EXPECT_EQ(3, id);
	}
}

TEST(MysqlSubmitService, NextInsertId)
{
	MysqlSubmitService service;

	uint64_t id = service.GetNextInsertId("user");

	EXPECT_EQ(4, id);
}

TEST(MysqlSubmitService, Select)
{
	MysqlSubmitService service;

	auto datatable = service.Query("select * from user");

	for(const auto& item: datatable)
	{
		for(const auto& elem: item)
		{
			if(elem.second.has_value())
				std::cout << elem.first << ":" << elem.second.value() << std::endl;
			else
				std::cout << elem.first << ":null" << std::endl;
		}
	}
}
