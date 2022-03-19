#include <gtest/gtest.h>
#include "../src/CheckService.hpp"

class CheckServiceTest: public testing::Test
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

		mysqlService.ExecuteCommand("insert into wareHouse(id, name) values(99, 'testWareHouse')");
		mysqlService.ExecuteCommand("insert into material(id, name) values(101, 'AMaterial')");
		mysqlService.ExecuteCommand("insert into material(id, name) values(102, 'BMaterial')");
		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values(201, 99, 101, 12.45, 0)");
		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values(202, 99, 102, 30, 1)");
		mysqlService.ExecuteCommand("insert into checkIn(id, itemInventoryId, number, time) values(301, 202, 1, '2000-01-01 12:07:49')");
		mysqlService.ExecuteCommand("insert into checkIn(id, itemInventoryId, number, time) values(302, 202, 1, '2000-01-02 20:00:01')");
		mysqlService.ExecuteCommand("insert into checkOut(id, itemInventoryId, number, time) values(401, 202, 1, '2000-01-05 07:35:22')");
	}

	inline virtual void TearDown() override
	{

	}
};

TEST_F(CheckServiceTest, Get)
{	
	CheckService checkService;

	const std::vector<CheckView> views = checkService.GetCheck(99, "2000-01-01 00:00:00", "2000-01-02 00:00:00");

	EXPECT_EQ(1, views.size());
	EXPECT_STREQ("BMaterial", views.front().name.c_str());
	EXPECT_EQ(30, views.front().cost);
	EXPECT_EQ(1, views.front().checkIn);
	EXPECT_EQ(0, views.front().checkOut);
}

TEST_F(CheckServiceTest, GetEmpty)
{	
	CheckService checkService;

	const std::vector<CheckView> views = checkService.GetCheck(99, "1999-01-01 00:00:00", "1999-01-02 00:00:00");

	EXPECT_EQ(0, views.size());
}

TEST_F(CheckServiceTest, GetWithUnVaildDateTime)
{
	CheckService checkService;


	const std::vector<CheckView> views = checkService.GetCheck(99, "asd", "asd");

	EXPECT_EQ(0, views.size());
}

TEST_F(CheckServiceTest, GetDetail)
{
	CheckService checkService;

	const std::vector<CheckDetailView> views = checkService.GetCheckDetail(202, "2000-01-01 00:00:00", "2000-01-02 00:00:00"); 

	ASSERT_EQ(1, views.size());
	EXPECT_EQ(1, views.front().number);
	EXPECT_STREQ("2000-01-01 12:07:49", views.front().time.c_str());
}

TEST_F(CheckServiceTest, GetDetailEmpty)
{
	CheckService checkService;

	const std::vector<CheckDetailView> views = checkService.GetCheckDetail(202, "1997-01-01 00:00:00", "1997-01-02 00:00:00"); 

	EXPECT_EQ(0, views.size());
}

TEST_F(CheckServiceTest, CancelCheckIn)
{
	CheckService checkService;
	MysqlService mysqlService;

	const auto oldDataTable = mysqlService.Query("select * from checkIn where id = ?", 301);

	const std::vector<std::string> sql = checkService.GetCancelCheckInSql(301);

	mysqlService.ExecuteCommandWithTran(sql);

	const auto dataTable = mysqlService.Query("select * from checkIn where id = ?", 301);

	EXPECT_EQ(1, oldDataTable.size());
	EXPECT_EQ(0, dataTable.size());
}

TEST_F(CheckServiceTest, CancelCheckOut)
{
	CheckService checkService;
	MysqlService mysqlService;

	const auto oldDataTable = mysqlService.Query("select * from checkOut where id = ?", 401);

	const std::vector<std::string> sql = checkService.GetCancelCheckOutSql(401);

	mysqlService.ExecuteCommandWithTran(sql);

	const auto dataTable = mysqlService.Query("select * from checkIn where id = ?", 401);

	EXPECT_EQ(1, oldDataTable.size());
	EXPECT_EQ(0, dataTable.size());
}

TEST_F(CheckServiceTest, CancelCheckInOutOfStock)
{
	CheckService checkService;
	MysqlService mysqlService;

	std::vector<std::string> sql(checkService.GetCancelCheckInSql(301));

	mysqlService.ExecuteCommandWithTran(sql);
	EXPECT_ANY_THROW
	({
		sql = checkService.GetCancelCheckInSql(302);

		mysqlService.ExecuteCommandWithTran(sql);
	});
}
