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

		mysqlService.ExecuteCommand("insert into material(id, name) values(101, 'AMaterial')");
	}

	virtual void TearDown()
	{
			
	}
};

TEST_F(MaterialServiceTest, GetMaterialByName)
{
	MaterialService materialService;

	const std::optional<Material> material = materialService.GetMaterial("AMaterial");

	EXPECT_TRUE(material.has_value());
	EXPECT_EQ(101, material->id);
	EXPECT_STREQ("AMaterial", material->name.c_str());
}

TEST_F(MaterialServiceTest, GetMaterialById)
{
	MaterialService materialService;

	const std::optional<Material> material = materialService.GetMaterial(101);

	EXPECT_TRUE(material.has_value());
	EXPECT_EQ(101, material->id);
	EXPECT_STREQ("AMaterial", material->name.c_str());
}

TEST_F(MaterialServiceTest, AddMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	const int nextInsertId = mysqlService.GetNextInsertId("material");
	const std::vector<std::string> sql = materialService.GetAddMaterialSql(nextInsertId, "newAddMaterial");
	mysqlService.ExecuteCommandWithTran(sql);

	std::optional<Material> material(materialService.GetMaterial(nextInsertId));

	EXPECT_TRUE(material.has_value());
	EXPECT_EQ(nextInsertId, material->id);
	EXPECT_STREQ("newAddMaterial", material->name.c_str());

	material = materialService.GetMaterial("newAddMaterial");

	EXPECT_TRUE(material.has_value());
	EXPECT_EQ(nextInsertId, material->id);
	EXPECT_STREQ("newAddMaterial", material->name.c_str());
}

TEST_F(MaterialServiceTest, AddRepeatNameMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	const int nextInsertId = mysqlService.GetNextInsertId("material");
	EXPECT_ANY_THROW
	({
		const std::vector<std::string> sql = materialService.GetAddMaterialSql(nextInsertId, "AMaterial");
	});
}

TEST_F(MaterialServiceTest, AddNullNameMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	const int nextInsertId = mysqlService.GetNextInsertId("material");

	EXPECT_ANY_THROW
	({
		const std::vector<std::string> sql = materialService.GetAddMaterialSql(nextInsertId, "");
	});
}

TEST_F(MaterialServiceTest, EditMaterial)
{
	MaterialService materialService;
	MysqlService mysqlService;

	mysqlService.ExecuteCommandWithTran(materialService.GetEditMaterialSql(101, "editMaterial"));

	const std::optional<Material> view = materialService.GetMaterial(101);

	ASSERT_TRUE(view.has_value());
	EXPECT_STREQ("editMaterial", view->name.c_str());
	EXPECT_EQ(101, view->id);
}

TEST_F(MaterialServiceTest, EditMaterialNullName)
{
	MaterialService materialService;
	MysqlService mysqlService;

	EXPECT_ANY_THROW
	({
		materialService.GetEditMaterialSql(101, "");
	});
}

TEST_F(MaterialServiceTest, EditMaterialEmptyId)
{
	MaterialService materialService;
	MysqlService mysqlService;

	EXPECT_ANY_THROW
	({
		materialService.GetEditMaterialSql(9999, "asdasd");
	});
}
