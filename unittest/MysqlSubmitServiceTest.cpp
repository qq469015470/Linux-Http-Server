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

		const std::string sqlCmd = "INSERT INTO user(name) VALUES('aaa')"; 

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
		std::cout << "id:" << *(int*)(item.at("id").value().data()) << std::endl;
		std::cout << "name:" << item.at("name").value().data() << std::endl;
	}

	EXPECT_EQ(2, datatable.size());
}

TEST(MysqlSubmitService, insertParam)
{
	MysqlSubmitService service;

	std::string name("param");
	int a = 123;

	service.ExecuteCommand("insert into user(id, name) values(?, ?)", a, name);

	auto datatable = service.Query("select * from user where id = ? and name = ?", a, name);

	for(const auto& item: datatable)
	{
		std::cout << "id:" << *(int*)(item.at("id").value().data()) << std::endl;
		std::cout << "name:" << item.at("name").value().data() << std::endl;
	}

	EXPECT_EQ(1, datatable.size());
}
