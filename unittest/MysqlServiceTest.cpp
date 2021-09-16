#include <gtest/gtest.h>
#include "../src/MysqlService.hpp"

TEST(MysqlServiceTest, DropTable)
{
	MysqlService service;

	const std::string sqlCmd = "DROP TABLE IF EXISTS `user`";

	service.ExecuteCommand(sqlCmd);
}

TEST(MysqlServiceTest, CreateTable)
{
	MysqlService service;

	const std::string sqlCmd = 
		"CREATE TABLE IF NOT EXISTS `user` ("
  			"`id` int NOT NULL AUTO_INCREMENT,"
  			"`name` varchar(20) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,"
  			"PRIMARY KEY (`id`)"
  		");";

	service.ExecuteCommand(sqlCmd);
}


TEST(MysqlServiceTest, Insert)
{
	{
		MysqlService service;

		const std::string sqlCmd = "INSERT INTO user(id, name) VALUES(2, '测试')"; 

		uint64_t id = service.ExecuteCommand(sqlCmd);

		EXPECT_EQ(2, id);
	}

	{
		MysqlService service;

		const std::string sqlCmd = "INSERT INTO user(name) VALUES('aaa')"; 

		uint64_t id = service.ExecuteCommand(sqlCmd);
		EXPECT_EQ(3, id);
	}
}

TEST(MysqlServiceTest, NextInsertId)
{
	MysqlService service;

	uint64_t id = service.GetNextInsertId("user");

	EXPECT_EQ(4, id);
}

TEST(MysqlServiceTest, Select)
{
	MysqlService service;

	auto datatable = service.Query("select * from user");

	for(const auto& item: datatable)
	{
		std::cout << "id:" << *(int*)(item.at("id").value().data()) << std::endl;
		std::cout << "name:" << item.at("name").value().data() << std::endl;
	}

	EXPECT_EQ(2, datatable.size());
}

TEST(MysqlServiceTest, insertParam)
{
	MysqlService service;

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

TEST(MysqlServiceTest, GetSafeSqlString)
{
	MysqlService service;

	const std::string before = "asd' or 1 == 1";
	const std::string result = service.GetSafeSqlString(before);

	std::cout << "beforesql:" << before << "|" << before.size() << std::endl;
	std::cout << "safesql:" << result << "|" << result.size() <<  std::endl;

	EXPECT_STRNE(result.data(), before.data());
}
