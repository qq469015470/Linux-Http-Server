#include <gtest/gtest.h>
#include "../src/WareHouseService.hpp"

class WareHouseServiceTest: public testing::Test
{
protected:
	inline virtual void SetUp() override
	{
		MysqlService mysqlService;

		mysqlService.ExecuteCommand("delete from checkIn");
		mysqlService.ExecuteCommand("delete from checkOut");
		mysqlService.ExecuteCommand("delete from itemInventory");
		mysqlService.ExecuteCommand("delete from material");
		mysqlService.ExecuteCommand("delete from wareHouse");

		mysqlService.ExecuteCommand("insert into wareHouse(id, name) values('e0b0e624-aa61-11ec-acff-000c29910818', 'test1')");
		mysqlService.ExecuteCommand("insert into wareHouse(id, name) values('e1de2dd3-aa61-11ec-acff-000c29910818', 'test2')");
	}

	inline virtual void TearDown() override
	{

	}
};

TEST_F(WareHouseServiceTest, GetWareHouse)
{
	WareHouseService wareHouseService;

	std::vector<WareHouse> wareHouses = wareHouseService.GetWareHouses();

	EXPECT_EQ(2, wareHouses.size());

	EXPECT_EQ("e0b0e624-aa61-11ec-acff-000c29910818", wareHouses.at(0).id);
	EXPECT_EQ("e1de2dd3-aa61-11ec-acff-000c29910818", wareHouses.at(1).id);

	EXPECT_STREQ("test1", wareHouses.at(0).name.c_str());
	EXPECT_STREQ("test2", wareHouses.at(1).name.c_str());
}

TEST_F(WareHouseServiceTest, Add)
{
	WareHouseService wareHouseService;

	wareHouseService.AddWareHouse("newWareHouse");

	MysqlService mysqlService;

	auto dataTable = mysqlService.Query("select * from wareHouse where name = ?", "newWareHouse");

	EXPECT_EQ(1, dataTable.size());
	EXPECT_STREQ("newWareHouse", dataTable.front().at("name")->data());
}

TEST_F(WareHouseServiceTest, AddNullName)
{
	WareHouseService wareHouseService;

	EXPECT_ANY_THROW
	({
		wareHouseService.AddWareHouse("");
	});
}

TEST_F(WareHouseServiceTest, AddRepeatName)
{
	WareHouseService wareHouseService;

	EXPECT_ANY_THROW
	({
		wareHouseService.AddWareHouse("test1");
	});
}
