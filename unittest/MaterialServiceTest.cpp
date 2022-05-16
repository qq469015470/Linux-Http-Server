#include <gtest/gtest.h>
#include "../src/MysqlService.hpp"
#include "../src/MaterialService.hpp"

class MaterialServiceTest: public testing::Test
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

		mysqlService.ExecuteCommand("insert into material(id, name) values('ad290436-aa66-11ec-acff-000c29910818', 'AMaterial')");
		mysqlService.ExecuteCommand("insert into material(id, name) values('289e7829-c244-11ec-9f69-00163e15a7e7', 'book')");
	}

	virtual void TearDown()
	{
			
	}
};

TEST_F(MaterialServiceTest, GetMaterialByName)
{
	MaterialService materialService;

	const std::optional<Material> material = materialService.GetMaterialByName("AMaterial");

	EXPECT_TRUE(material.has_value());
	EXPECT_STREQ("ad290436-aa66-11ec-acff-000c29910818", material->id.c_str());
	EXPECT_STREQ("AMaterial", material->name.c_str());
}

TEST_F(MaterialServiceTest, ContainsMaterialByName)
{
	MaterialService materialService;

	const std::vector<Material> materials = materialService.ContainsMaterialByName("A");

	ASSERT_EQ(1, materials.size());
	EXPECT_STREQ("ad290436-aa66-11ec-acff-000c29910818", materials.front().id.c_str());
	EXPECT_STREQ("AMaterial", materials.front().name.c_str());
}

TEST_F(MaterialServiceTest, ContainsMaterialByNameInPercentChar)
{
	MaterialService materialService;

	const std::vector<Material> materials = materialService.ContainsMaterialByName("%");

	ASSERT_EQ(0, materials.size());
}

TEST_F(MaterialServiceTest, GetMaterialById)
{
	MaterialService materialService;

	const std::optional<Material> material = materialService.GetMaterialById("ad290436-aa66-11ec-acff-000c29910818");

	EXPECT_TRUE(material.has_value());
	EXPECT_STREQ("ad290436-aa66-11ec-acff-000c29910818", material->id.c_str());
	EXPECT_STREQ("AMaterial", material->name.c_str());
}

TEST_F(MaterialServiceTest, AddMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	const std::string nextInsertId = mysqlService.GetUUID();
	const std::vector<std::string> sql = materialService.GetAddMaterialSql(nextInsertId, "newAddMaterial");
	mysqlService.ExecuteCommandWithTran(sql);

	std::optional<Material> material(materialService.GetMaterialById(nextInsertId));

	EXPECT_TRUE(material.has_value());
	EXPECT_STREQ("newAddMaterial", material->name.c_str());

	material = materialService.GetMaterialByName("newAddMaterial");

	EXPECT_TRUE(material.has_value());
	EXPECT_STREQ(nextInsertId.c_str(), material->id.c_str());
	EXPECT_STREQ("newAddMaterial", material->name.c_str());
}

TEST_F(MaterialServiceTest, AddRepeatNameMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	const std::string nextInsertId = mysqlService.GetUUID();
	EXPECT_ANY_THROW
	({
		const std::vector<std::string> sql = materialService.GetAddMaterialSql(nextInsertId, "AMaterial");
	});
}

TEST_F(MaterialServiceTest, AddNullNameMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	const std::string nextInsertId = mysqlService.GetUUID();

	EXPECT_ANY_THROW
	({
		const std::vector<std::string> sql = materialService.GetAddMaterialSql(nextInsertId, "");
	});
}

TEST_F(MaterialServiceTest, EditMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	mysqlService.ExecuteCommandWithTran(materialService.GetEditMaterialSql("ad290436-aa66-11ec-acff-000c29910818", "editMaterial"));

	const std::optional<Material> view = materialService.GetMaterialById("ad290436-aa66-11ec-acff-000c29910818");

	ASSERT_TRUE(view.has_value());
	EXPECT_STREQ("editMaterial", view->name.c_str());
	EXPECT_STREQ("ad290436-aa66-11ec-acff-000c29910818", view->id.c_str());
}

TEST_F(MaterialServiceTest, EditMaterialRepeatName)
{
	MaterialService materialService;


	EXPECT_THROW
	({
		materialService.GetEditMaterialSql("ad290436-aa66-11ec-acff-000c29910818", "book");
	}, std::logic_error);
}

TEST_F(MaterialServiceTest, EditMaterialNullName)
{
	MaterialService materialService;
	MysqlService mysqlService;

	EXPECT_ANY_THROW
	({
		materialService.GetEditMaterialSql("ad290436-aa66-11ec-acff-000c29910818", "");
	});
}

TEST_F(MaterialServiceTest, EditMaterialEmptyId)
{
	MaterialService materialService;
	MysqlService mysqlService;

	EXPECT_ANY_THROW
	({
		materialService.GetEditMaterialSql("9999", "asdasd");
	});
}
