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


TEST(JsonObj, ParseJsonArray2)
{
	web::JsonObj obj;

	std::string json = "[{\"stock\": 1230.000000,\"name\": \"' 1 ==  1\",\"materialId\": 105,\"wareHouseId\": 99,\"price\": 34.100000,\"id\": 74},{\"stock\": 1230.000000,\"name\": \"'拉拉\",\"materialId\": 105,\"wareHouseId\": 99,\"price\": 34.100000,\"id\": 74}]";

	obj = web::JsonObj::ParseJson(json); 

	std::cout << obj.ToJson() << std::endl;
	EXPECT_EQ(2, obj.GetArraySize());
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

TEST(JsonObj, SetArray)
{
       	web::JsonObj res;
       	
       	for(int i = 0; i < 10; i++)
       	{
       		web::JsonObj temp;

       		temp["roomId"] = "id";
       		temp["name"] = "name";
       		temp["ip"] = std::to_string(i);
	
       		res.Push(std::move(temp));
       	}


	EXPECT_EQ(res.GetArraySize(), 10);
	for(int i = 0; i < 10; i++)
	{
		EXPECT_EQ(res[i]["roomId"].ToString(), "id");
		EXPECT_EQ(res[i]["name"].ToString(), "name");
		EXPECT_EQ(res[i]["ip"].ToString(), std::to_string(i));
	}
}

TEST(JsonObj, TurnChar)
{
	web::JsonObj res;

	res["name"] = "\"双引号 +\"";

	std::cout << res.ToJson() << std::endl;
	std::cout << res["name"].ToString() << std::endl;

	EXPECT_STREQ("\"\\\"双引号 +\\\"\"", res["name"].ToJson().data());

	web::JsonObj temp = web::JsonObj::ParseJson(res.ToJson());

	std::cout << "res to json:" << std::endl;
	std::cout << res.ToJson() << std::endl;
	std::cout << res["name"].ToString() << std::endl;
	std::cout << res["name"].ToJson() << std::endl;
	std::cout << "temp to json:" << std::endl;
	std::cout << temp.ToJson() << std::endl;
	std::cout << temp["name"].ToString() << std::endl;
	std::cout << temp["name"].ToJson() << std::endl;

	EXPECT_STREQ("\"\\\"双引号 +\\\"\"", temp["name"].ToJson().data());
}
