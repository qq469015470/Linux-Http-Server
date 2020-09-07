#include <gtest/gtest.h>
#include "../src/web.h"

TEST(JsonObj, Parse)
{
	std::string json = "age=30&name=abc";

	auto result = web::JsonObj::ParseFormData(std::move(json));
	
	EXPECT_EQ(result["age"].ToString(), "30");
	EXPECT_EQ(result["name"].ToString(), "abc");
}

TEST(JsonObj, ParseArray)
{
	std::string json = "age%5B%5D=30&age%5B%5D=40";

	auto result = web::JsonObj::ParseFormData(std::move(json));

	EXPECT_EQ(result["age"][0].ToString(), "30");
	EXPECT_EQ(result["age"][1].ToString(), "40");
}

TEST(JsonObj, ParseAll)
{
	std::string json = "name=aaa&age%5B%5D=10&age%5B%5D=20&source=100";

	auto result = web::JsonObj::ParseFormData(std::move(json));

	EXPECT_EQ(result["name"].ToString(), "aaa");
	EXPECT_EQ(result["age"][0].ToString(), "10");
	EXPECT_EQ(result["age"][1].ToString(), "20");
	EXPECT_EQ(result["source"].ToString(), "100");
}
