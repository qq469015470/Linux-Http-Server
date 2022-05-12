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

		mysqlService.ExecuteCommand("insert into wareHouse(id, name) values('b39f64c4-ab37-11ec-acff-000c29910818', 'testWareHouse')");
		mysqlService.ExecuteCommand("insert into material(id, name) values('d87ad120-ab37-11ec-acff-000c29910818', 'AMaterial')");
		mysqlService.ExecuteCommand("insert into material(id, name) values('a4e26329-ab37-11ec-acff-000c29910818', 'BMaterial')");
		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values('e358b169-ab37-11ec-acff-000c29910818', 'b39f64c4-ab37-11ec-acff-000c29910818', 'd87ad120-ab37-11ec-acff-000c29910818', 12.45, 0)");
		mysqlService.ExecuteCommand("insert into itemInventory(id, wareHouseId, materialId, cost, stock) values('f485858a-ab37-11ec-acff-000c29910818', 'b39f64c4-ab37-11ec-acff-000c29910818', 'a4e26329-ab37-11ec-acff-000c29910818', 30, 1)");
		mysqlService.ExecuteCommand("insert into checkIn(id, itemInventoryId, number, time) values('01294628-ab38-11ec-acff-000c29910818', 'f485858a-ab37-11ec-acff-000c29910818', 1, '2000-01-01 12:07:49')");
		mysqlService.ExecuteCommand("insert into checkIn(id, itemInventoryId, number, time) values('0cbd02d5-ab38-11ec-acff-000c29910818', 'f485858a-ab37-11ec-acff-000c29910818', 1, '2000-01-02 20:00:01')");
		mysqlService.ExecuteCommand("insert into checkOut(id, itemInventoryId, number, time) values('149c5384-ab38-11ec-acff-000c29910818', 'f485858a-ab37-11ec-acff-000c29910818', 1, '2000-01-05 07:35:22')");
	}

	inline virtual void TearDown() override
	{

	}
};

TEST_F(CheckServiceTest, Get)
{	
	CheckService checkService;

	const std::vector<CheckView> views = checkService.GetCheck("b39f64c4-ab37-11ec-acff-000c29910818", "2000-01-01 00:00:00", "2000-01-02 00:00:00");

	EXPECT_EQ(1, views.size());
	EXPECT_STREQ("BMaterial", views.front().name.c_str());
	EXPECT_EQ(30, views.front().cost);
	EXPECT_EQ(1, views.front().checkIn);
	EXPECT_EQ(0, views.front().checkOut);
}

TEST_F(CheckServiceTest, GetEmpty)
{	
	CheckService checkService;

	const std::vector<CheckView> views = checkService.GetCheck("b39f64c4-ab37-11ec-acff-000c29910818", "1999-01-01 00:00:00", "1999-01-02 00:00:00");

	EXPECT_EQ(0, views.size());
}

TEST_F(CheckServiceTest, GetWithUnVaildDateTime)
{
	CheckService checkService;


	const std::vector<CheckView> views = checkService.GetCheck("b39f64c4-ab37-11ec-acff-000c29910818", "asd", "asd");

	EXPECT_EQ(0, views.size());
}

TEST_F(CheckServiceTest, GetDetail)
{
	CheckService checkService;

	const std::vector<CheckDetailView> views = checkService.GetCheckDetail("f485858a-ab37-11ec-acff-000c29910818", "2000-01-01 00:00:00", "2000-01-02 00:00:00"); 

	ASSERT_EQ(1, views.size());
	EXPECT_EQ(1, views.front().number);
	EXPECT_STREQ("2000-01-01 12:07:49", views.front().time.c_str());
}

TEST_F(CheckServiceTest, GetDetailEmpty)
{
	CheckService checkService;

	const std::vector<CheckDetailView> views = checkService.GetCheckDetail("f485858a-ab37-11ec-acff-000c29910818", "1997-01-01 00:00:00", "1997-01-02 00:00:00"); 

	EXPECT_EQ(0, views.size());
}

TEST_F(CheckServiceTest, CancelCheckIn)
{
	CheckService checkService;
	MysqlService mysqlService;

	const auto oldDataTable = mysqlService.Query("select * from checkIn where id = ?", "01294628-ab38-11ec-acff-000c29910818");

	const std::vector<std::string> sql = checkService.GetCancelCheckInSql("01294628-ab38-11ec-acff-000c29910818");

	mysqlService.ExecuteCommandWithTran(sql);

	const auto dataTable = mysqlService.Query("select * from checkIn where id = ?", "01294628-ab38-11ec-acff-000c29910818");

	EXPECT_EQ(1, oldDataTable.size());
	EXPECT_EQ(0, dataTable.size());
}

TEST_F(CheckServiceTest, CancelCheckOut)
{
	CheckService checkService;
	MysqlService mysqlService;

	const auto oldDataTable = mysqlService.Query("select * from checkOut where id = ?", "149c5384-ab38-11ec-acff-000c29910818");

	const std::vector<std::string> sql = checkService.GetCancelCheckOutSql("149c5384-ab38-11ec-acff-000c29910818");

	mysqlService.ExecuteCommandWithTran(sql);

	const auto dataTable = mysqlService.Query("select * from checkIn where id = ?", "149c5384-ab38-11ec-acff-000c29910818");

	EXPECT_EQ(1, oldDataTable.size());
	EXPECT_EQ(0, dataTable.size());
}

TEST_F(CheckServiceTest, CancelCheckInOutOfStock)
{
	CheckService checkService;
	MysqlService mysqlService;

	std::vector<std::string> sql(checkService.GetCancelCheckInSql("01294628-ab38-11ec-acff-000c29910818"));

	mysqlService.ExecuteCommandWithTran(sql);
	EXPECT_ANY_THROW
	({
		sql = checkService.GetCancelCheckInSql("0cbd02d5-ab38-11ec-acff-000c29910818");

		mysqlService.ExecuteCommandWithTran(sql);
	});
}

TEST_F(CheckServiceTest, GetCheckNote)
{
	CheckService checkService;

	std::vector<CheckNoteView> views(checkService.GetCheckNoteView("b39f64c4-ab37-11ec-acff-000c29910818", "2000-01-02"));
	std::sort(views.begin(), views.end(), [](const CheckNoteView& a, const CheckNoteView& b)
	{
		return a.stock > b.stock;
	});

	ASSERT_EQ(2, views.size());
	EXPECT_STREQ("f485858a-ab37-11ec-acff-000c29910818", views.at(0).id.c_str());
	EXPECT_EQ(2, views.at(0).stock);
	EXPECT_STREQ("e358b169-ab37-11ec-acff-000c29910818", views.at(1).id.c_str());
	EXPECT_EQ(0, views.at(1).stock);
}
