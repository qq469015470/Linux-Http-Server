#include <gtest/gtest.h>
#include "../src/web.h"

TEST(JsonObj, ParseFormData)
{
	std::string formData = "age=30&name=abc";

	auto result = web::JsonObj::ParseFormData(std::move(formData));
	
	EXPECT_EQ(result["age"].ToString(), "30");
	EXPECT_EQ(result["name"].ToString(), "abc");
}

TEST(JsonObj, ParseFormData2)
{
	std::string formData = "token=123";

	auto result = web::JsonObj::ParseFormData(std::move(formData));
	
	EXPECT_EQ(result["token"].ToString(), "123");
}

TEST(JsonObj, ParseFormDataArray)
{
	std::string formData = "age%5B%5D=30&age%5B%5D=40";

	auto result = web::JsonObj::ParseFormData(std::move(formData));

	EXPECT_EQ(result["age"][0].ToString(), "30");
	EXPECT_EQ(result["age"][1].ToString(), "40");
}

TEST(JsonObj, ParseFormDataAll)
{
	std::string formData = "name=aaa&age%5B%5D=10&age%5B%5D=20&source=100";

	auto result = web::JsonObj::ParseFormData(std::move(formData));

	EXPECT_EQ(result["name"].ToString(), "aaa");
	EXPECT_EQ(result["age"][0].ToString(), "10");
	EXPECT_EQ(result["age"][1].ToString(), "20");
	EXPECT_EQ(result["source"].ToString(), "100");
}

TEST(JsonObj, ParseJson)
{
	std::string json = "{\"firstName\": \"Brett\", \"lastName\": \"McLaughlin\",\"asd\":    \"aa\"}";

	auto result = web::JsonObj::ParseJson(json);

	EXPECT_EQ(result["firstName"].ToString(), "Brett");
	EXPECT_EQ(result["lastName"].ToString(), "McLaughlin");
	EXPECT_EQ(result["asd"].ToString(), "aa");
}

TEST(JsonObj, ParseJsonArray)
{
	std::string json = "{\"people\":[{\"firstName\": \"Brett\",\"lastName\":\"McLaughlin\"},{\"firstName\":\"Jason\",\"lastName\":\"Hunter\"}]}";

	auto result = web::JsonObj::ParseJson(json);

	EXPECT_EQ(result["people"][0]["firstName"].ToString(), "Brett");
	EXPECT_EQ(result["people"][0]["lastName"].ToString(), "McLaughlin");
	EXPECT_EQ(result["people"][1]["firstName"].ToString(), "Jason");
	EXPECT_EQ(result["people"][1]["lastName"].ToString(), "Hunter");
}

TEST(JsonObj, ParseJsonObject)
{
	std::string json = "{ \"name\":\"Bill Gates\", \"age\":62, \"cars\": {\"car1\":\"Porsche\", \"car2\":\"BMW\", \"car3\":\"Volvo\"}}";
	auto result = web::JsonObj::ParseJson(json);

	EXPECT_EQ(result["name"].ToString(), "Bill Gates");
	EXPECT_EQ(result["age"].ToDouble(), 62);
	EXPECT_EQ(result["cars"]["car1"].ToString(), "Porsche");
	EXPECT_EQ(result["cars"]["car2"].ToString(), "BMW");
	EXPECT_EQ(result["cars"]["car3"].ToString(), "Volvo");
}
