#include <gtest/gtest.h>
#include "../src/MysqlService.hpp"
#include "../src/ItemInventoryService.hpp"

class ItemInventoryServiceTest: public testing::Test
{
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

		mysqlService.ExecuteCommand("insert into material(id, name) values(101, 'AMaterial')");
		mysqlService.ExecuteCommand("insert into material(id, name) values(102, 'BMaterial')");
		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values(201, 99, 101, 12.45, 0)");
		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values(202, 99, 102, 30, 1)");
		mysqlService.ExecuteCommand("insert into checkIn(id, itemInventoryId, number, time) values(301, 202, 1, '2000-01-01 12:07:49')");
	}

	virtual void TearDown()
	{

	}
};

TEST_F(ItemInventoryServiceTest, GetItemInventory)
{
	ItemInventoryService itemInventoryService;

	const std::optional<ItemInventoryView> view = itemInventoryService.GetById(201);

	EXPECT_EQ(201, view->id);
	EXPECT_EQ(99, view->wareHouseId);
	EXPECT_EQ(101, view->materialId);
	EXPECT_STREQ("AMaterial", view->name.c_str());
	EXPECT_EQ(12.45, view->price);
	EXPECT_EQ(0, view->stock);
}

TEST_F(ItemInventoryServiceTest, GetItemInvenotoryList)
{
	ItemInventoryService itemInventoryService;

	const std::vector<ItemInventoryView> views = itemInventoryService.GetByWareHouseId(99);

	EXPECT_EQ(2, views.size());

	EXPECT_EQ(201, views.at(0).id);
	EXPECT_EQ(99, views.at(0).wareHouseId);
	EXPECT_EQ(101, views.at(0).materialId);
	EXPECT_STREQ("AMaterial", views.at(0).name.c_str());
	EXPECT_EQ(12.45, views.at(0).price);
	EXPECT_EQ(0, views.at(0).stock);

	EXPECT_EQ(202, views.at(1).id);
	EXPECT_EQ(99, views.at(1).wareHouseId);
	EXPECT_EQ(102, views.at(1).materialId);
	EXPECT_STREQ("BMaterial", views.at(1).name.c_str());
	EXPECT_EQ(30, views.at(1).price);
	EXPECT_EQ(1, views.at(1).stock);
}

TEST_F(ItemInventoryServiceTest, AddItemInventory)
{
	ItemInventoryService itemInventoryService;

	std::vector<std::string> sql(itemInventoryService.GetAddItemInventorySql("testMaterial",  99, 12.34));

	for(const auto& item: sql)
		std::cout << item << std::endl;
	
	MysqlService mysqlService;

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from itemInventory");
	EXPECT_EQ(3, datatable.size());

	datatable = mysqlService.Query("select * from material");
	EXPECT_EQ(3, datatable.size());
}


TEST_F(ItemInventoryServiceTest, AddItemInventory_InExistMaterial)
{
	MysqlService mysqlService;

	mysqlService.ExecuteCommand("insert into material(id, name) values(999, 'existMaterial')");
	ItemInventoryService itemInventoryService;

	std::vector<std::string> sql(itemInventoryService.GetAddItemInventorySql("existMaterial",  99, 12.34));

	for(const auto& item: sql)
		std::cout << item << std::endl;
	

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from itemInventory");
	EXPECT_EQ(3, datatable.size());
	datatable = mysqlService.Query("select * from material;");
	EXPECT_EQ(3, datatable.size());
}

TEST_F(ItemInventoryServiceTest, CheckIn)
{
	MysqlService mysqlService;
	ItemInventoryService itemInventoryService;	

	std::vector<std::string> sql(itemInventoryService.GetCheckInSql(201, 2));

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from checkIn where itemInventoryId = 201;");

	EXPECT_EQ(1, datatable.size());
	EXPECT_EQ(2, *reinterpret_cast<double*>(datatable.front().at("number")->data()));
}

TEST_F(ItemInventoryServiceTest, CheckOut)
{
	MysqlService mysqlService;
	ItemInventoryService itemInventoryService;

	std::vector<std::string> sql(itemInventoryService.GetCheckOutSql(202, 1));

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from checkOut where itemInventoryId = 202;");

	EXPECT_EQ(1, datatable.size());
	EXPECT_EQ(1, *reinterpret_cast<double*>(datatable.front().at("number")->data()));
}

TEST_F(ItemInventoryServiceTest, Edit)
{
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

	const std::vector<std::string> sql = itemInventoryService.GetEditItemInventorySql(202, "editNewMaterialName", 666.6);

	mysqlService.ExecuteCommandWithTran(sql);

	const std::optional<ItemInventoryView> view = itemInventoryService.GetById(202);

	ASSERT_TRUE(view.has_value());
	EXPECT_EQ(202, view->id);
	EXPECT_STREQ("editNewMaterialName", view->name.c_str());
	EXPECT_EQ(99, view->wareHouseId);
	EXPECT_EQ(666.6, view->price);
	EXPECT_EQ(1, view->stock);
	EXPECT_EQ(102, view->materialId);
}

TEST_F(ItemInventoryServiceTest, EditMaterialNameRepeat)
{
	ItemInventoryService itemInventoryService;

	EXPECT_ANY_THROW
	({
		itemInventoryService.GetEditItemInventorySql(202, "AMaterial", 666.6);
	});
}

TEST_F(ItemInventoryServiceTest, EditEmptyMaterialName)
{
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

	EXPECT_ANY_THROW
	({
	 	itemInventoryService.GetEditItemInventorySql(202, "", 666.6);
	});
}
