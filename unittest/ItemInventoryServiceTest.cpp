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

		mysqlService.ExecuteCommand("insert into wareHouse(id, name) values('f09146a6-ab38-11ec-acff-000c29910818', 'test')");

		mysqlService.ExecuteCommand("insert into material(id, name) values('2474240c-ab39-11ec-acff-000c29910818', 'AMaterial')");
		mysqlService.ExecuteCommand("insert into material(id, name) values('32be680d-ab39-11ec-acff-000c29910818', 'anotherMaterial')");

		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values('44a71920-ab39-11ec-acff-000c29910818', 'f09146a6-ab38-11ec-acff-000c29910818', '2474240c-ab39-11ec-acff-000c29910818', 12.45, 0)");
		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values('5476a844-ab39-11ec-acff-000c29910818', 'f09146a6-ab38-11ec-acff-000c29910818', '32be680d-ab39-11ec-acff-000c29910818', 30, 1)");
		mysqlService.ExecuteCommand("insert into checkIn(id, itemInventoryId, number, time) values('6b16878e-ab39-11ec-acff-000c29910818', '5476a844-ab39-11ec-acff-000c29910818', 1, '2000-01-01 12:07:49')");
	}

	virtual void TearDown()
	{

	}
};

TEST_F(ItemInventoryServiceTest, GetItemInventory)
{
	ItemInventoryService itemInventoryService;

	const std::optional<ItemInventoryView> view = itemInventoryService.GetById("44a71920-ab39-11ec-acff-000c29910818");

	ASSERT_TRUE(view.has_value());
	EXPECT_STREQ("44a71920-ab39-11ec-acff-000c29910818", view->id.c_str());
	EXPECT_STREQ("f09146a6-ab38-11ec-acff-000c29910818", view->wareHouseId.c_str());
	EXPECT_STREQ("2474240c-ab39-11ec-acff-000c29910818", view->materialId.c_str());
	EXPECT_STREQ("AMaterial", view->name.c_str());
	EXPECT_EQ(12.45, view->price);
	EXPECT_EQ(0, view->stock);
}

TEST_F(ItemInventoryServiceTest, ContainsMaterialName)
{
	ItemInventoryService itemInventoryService;

	const std::vector<Material> materials = itemInventoryService.GetContainsMaterialName("f09146a6-ab38-11ec-acff-000c29910818", "anotherMaterial");

	ASSERT_EQ(1, materials.size());
	EXPECT_STREQ("32be680d-ab39-11ec-acff-000c29910818", materials.front().id.c_str());
	EXPECT_STREQ("anotherMaterial", materials.front().name.c_str());
}

TEST_F(ItemInventoryServiceTest, ContainsMaterialNameByPercentChar)
{
	ItemInventoryService itemInventoryService;
	const std::vector<Material> materials = itemInventoryService.GetContainsMaterialName("f09146a6-ab38-11ec-acff-000c29910818", "%");

	ASSERT_EQ(0, materials.size());
}

TEST_F(ItemInventoryServiceTest, GetItemInvenotoryList)
{
	ItemInventoryService itemInventoryService;

	const std::vector<ItemInventoryView> views = itemInventoryService.GetByWareHouseId("f09146a6-ab38-11ec-acff-000c29910818");

	EXPECT_EQ(2, views.size());

	EXPECT_STREQ("44a71920-ab39-11ec-acff-000c29910818", views.at(0).id.c_str());
	EXPECT_STREQ("f09146a6-ab38-11ec-acff-000c29910818", views.at(0).wareHouseId.c_str());
	EXPECT_STREQ("2474240c-ab39-11ec-acff-000c29910818", views.at(0).materialId.c_str());
	EXPECT_STREQ("AMaterial", views.at(0).name.c_str());
	EXPECT_EQ(12.45, views.at(0).price);
	EXPECT_EQ(0, views.at(0).stock);

	EXPECT_STREQ("5476a844-ab39-11ec-acff-000c29910818", views.at(1).id.c_str());
	EXPECT_STREQ("f09146a6-ab38-11ec-acff-000c29910818", views.at(1).wareHouseId.c_str());
	EXPECT_STREQ("32be680d-ab39-11ec-acff-000c29910818", views.at(1).materialId.c_str());
	EXPECT_STREQ("anotherMaterial", views.at(1).name.c_str());
	EXPECT_EQ(30, views.at(1).price);
	EXPECT_EQ(1, views.at(1).stock);
}

TEST_F(ItemInventoryServiceTest, AddItemInventory)
{
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

	std::vector<std::string> sql(itemInventoryService.GetAddItemInventorySql(mysqlService.GetUUID(), "testMaterial", "f09146a6-ab38-11ec-acff-000c29910818", 12.34));

	for(const auto& item: sql)
		std::cout << item << std::endl;
	
	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from itemInventory");
	EXPECT_EQ(3, datatable.size());

	datatable = mysqlService.Query("select * from material");
	EXPECT_EQ(3, datatable.size());
}

TEST_F(ItemInventoryServiceTest, CheckIn)
{
	MysqlService mysqlService;
	ItemInventoryService itemInventoryService;	

	std::vector<std::string> sql(itemInventoryService.GetCheckInSql("44a71920-ab39-11ec-acff-000c29910818", 2));

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from checkIn where itemInventoryId = '44a71920-ab39-11ec-acff-000c29910818';");

	EXPECT_EQ(1, datatable.size());
	EXPECT_EQ(2, std::stod(datatable.front().at("number")->data()));
}

TEST_F(ItemInventoryServiceTest, CheckOut)
{
	MysqlService mysqlService;
	ItemInventoryService itemInventoryService;

	std::vector<std::string> sql(itemInventoryService.GetCheckOutSql("5476a844-ab39-11ec-acff-000c29910818", 1));

	mysqlService.ExecuteCommandWithTran(sql);

	auto datatable = mysqlService.Query("select * from checkOut where itemInventoryId = '5476a844-ab39-11ec-acff-000c29910818';");

	EXPECT_EQ(1, datatable.size());
	EXPECT_EQ(1, std::stod(datatable.front().at("number")->data()));
}

TEST_F(ItemInventoryServiceTest, Edit)
{
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

	const std::vector<std::string> sql = itemInventoryService.GetEditItemInventorySql("5476a844-ab39-11ec-acff-000c29910818", "editNewMaterialName", 666.6);

	mysqlService.ExecuteCommandWithTran(sql);

	const std::optional<ItemInventoryView> view = itemInventoryService.GetById("5476a844-ab39-11ec-acff-000c29910818");

	ASSERT_TRUE(view.has_value());
	EXPECT_STREQ("5476a844-ab39-11ec-acff-000c29910818", view->id.c_str());
	EXPECT_STREQ("editNewMaterialName", view->name.c_str());
	EXPECT_STREQ("f09146a6-ab38-11ec-acff-000c29910818", view->wareHouseId.c_str());
	EXPECT_EQ(666.6, view->price);
	EXPECT_EQ(1, view->stock);
	EXPECT_STREQ("32be680d-ab39-11ec-acff-000c29910818", view->materialId.c_str());
}

TEST_F(ItemInventoryServiceTest, EditOnlyPrice)
{
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

	const std::vector<std::string> sql = itemInventoryService.GetEditItemInventorySql("5476a844-ab39-11ec-acff-000c29910818", "anotherMaterial", 666.6);

	mysqlService.ExecuteCommandWithTran(sql);

	const std::optional<ItemInventoryView> view = itemInventoryService.GetById("5476a844-ab39-11ec-acff-000c29910818");

	ASSERT_TRUE(view.has_value());
	EXPECT_STREQ("5476a844-ab39-11ec-acff-000c29910818", view->id.c_str());
	EXPECT_STREQ("anotherMaterial", view->name.c_str());
	EXPECT_STREQ("f09146a6-ab38-11ec-acff-000c29910818", view->wareHouseId.c_str());
	EXPECT_EQ(666.6, view->price);
	EXPECT_EQ(1, view->stock);
	EXPECT_STREQ("32be680d-ab39-11ec-acff-000c29910818", view->materialId.c_str());
}

TEST_F(ItemInventoryServiceTest, EditMaterialNameRepeat)
{
	ItemInventoryService itemInventoryService;

	EXPECT_ANY_THROW
	({
		itemInventoryService.GetEditItemInventorySql("5476a844-ab39-11ec-acff-000c29910818", "AMaterial", 666.6);
	});
}

TEST_F(ItemInventoryServiceTest, EditEmptyMaterialName)
{
	ItemInventoryService itemInventoryService;
	MysqlService mysqlService;

	EXPECT_ANY_THROW
	({
	 	itemInventoryService.GetEditItemInventorySql("5476a844-ab39-11ec-acff-000c29910818", "", 666.6);
	});
}
