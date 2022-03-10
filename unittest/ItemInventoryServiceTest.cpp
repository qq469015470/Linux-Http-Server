#include <gtest/gtest.h>
#include "../src/MysqlService.hpp"
#include "../src/ItemInventoryService.hpp"

class ItemInventoryServiceTest: public testing::Test
{
private:
	MysqlService mysqlService;

protected:
	virtual void SetUp()
	{
		MysqlService mysqlService;

		mysqlService.ExecuteCommand("delete from checkIn");
		mysqlService.ExecuteCommand("delete from checkOut");
		mysqlService.ExecuteCommand("delete from itemInventory");
		mysqlService.ExecuteCommand("delete from material");
		mysqlService.ExecuteCommand("delete from wareHouse");

		mysqlService.ExecuteCommand("insert into wareHouse(id, name) values(99, 'test')");
	}

	virtual void TearDown()
	{

	}
};


TEST_F(ItemInventoryServiceTest, AddItemInventory)
{
	ItemInventoryService itemInventoryService;

	std::vector<std::string> sql(itemInventoryService.GetAddItemInventorySql("testMaterial",  99, 12.34));

	for(const auto& item: sql)
		std::cout << item << std::endl;
	
	MysqlService mysqlService;

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from itemInventory");
	EXPECT_EQ(1, datatable.size());
}


TEST_F(ItemInventoryServiceTest, AddItemInventory_InExistMaterial)
{
	MysqlService mysqlService;
	ItemInventoryService itemInventoryService;

	mysqlService.ExecuteCommand("insert into material(id, name) values(99, 'testMaterial')");


	std::vector<std::string> sql(itemInventoryService.GetAddItemInventorySql("testMaterial",  99, 12.34));

	for(const auto& item: sql)
		std::cout << item << std::endl;
	

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from itemInventory");
	EXPECT_EQ(1, datatable.size());
}
