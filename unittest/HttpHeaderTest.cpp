#include <gtest/gtest.h>
#include "../src/web.h"

TEST(HttpHeader, ConvertHeader)
{
	const char* request = 
	//"GET / HTTP/1.1\r\n"
	"Host: www.baidu.com\r\n"
	"Connection: keep-alive\r\n"
	"Pragma: no-cache\r\n"
	"Cache-Control: no-cache\r\n"
	"Upgrade-Insecure-Requests: 1\r\n"
	"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36\r\n"
	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
	"Sec-Fetch-Site: none\r\n"
	"Sec-Fetch-Mode: navigate\r\n"
	"Sec-Fetch-User: ?1\r\n"
	"Sec-Fetch-Dest: document\r\n"
	"Accept-Encoding: gzip, deflate, br\r\n"
	"Accept-Language: en-US,en;q=0.9,zh;q=0.8,zh-CN;q=0.7\r\n"
	"Cookie: BIDUPSID=EE7F8035EAF85C3FAA8C0ED90AE33BBB; PSTM=1592191861; BAIDUID=EE7F8035EAF85C3F702D40EC7B98EE2B:FG=1; BD_UPN=123353; BDORZ=B490B5EBF6F3CD402E515D22BCDA1598; COOKIE_SESSION=6154037_0_3_3_0_3_0_0_3_2_1_0_0_0_0_0_0_0_1599478984%7C3%230_0_1599478984%7C1\r\n";

	web::HttpHeader header(request);

	EXPECT_EQ(header.GetContentLength(), 0);
	EXPECT_EQ(std::string(header.GetConnection()), "keep-alive");
	EXPECT_EQ(std::string(header.GetUpgrade()), "");
	EXPECT_EQ(std::string(header.GetCookie("BIDUPSID")), "EE7F8035EAF85C3FAA8C0ED90AE33BBB");
	EXPECT_EQ(std::string(header.GetCookie("PSTM")), "1592191861");
	EXPECT_EQ(std::string(header.GetCookie("BAIDUID")), "EE7F8035EAF85C3F702D40EC7B98EE2B:FG=1");
	EXPECT_EQ(std::string(header.GetCookie("BD_UPN")), "123353");
	EXPECT_EQ(std::string(header.GetCookie("BDORZ")), "B490B5EBF6F3CD402E515D22BCDA1598");
	EXPECT_EQ(std::string(header.GetCookie("COOKIE_SESSION")), "6154037_0_3_3_0_3_0_0_3_2_1_0_0_0_0_0_0_0_1599478984%7C3%230_0_1599478984%7C1");
	EXPECT_EQ(header.GetHttpAttrs().size(), 14);
}


TEST(HttpHeader, ConvertHeader2)
{
	std::vector<web::HttpAttr> attrs =
	{
		{"Host", "www.baidu.com"},
		{"Connection", "keep-alive"},
		{"Pragma", "no-cache"},
		{"Cache-Control", "no-cache"},
		{"Upgrade-Insecure-Requests", "1"},
		{"User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"},
		{"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9"},
		{"Sec-Fetch-Site", "none"},
		{"Sec-Fetch-Mode", "navigate"},
		{"Sec-Fetch-User", "?1"},
		{"Sec-Fetch-Dest", "document"},
		{"Accept-Encoding", "gzip, deflate, br"},
		{"Accept-Language", "en-US,en;q=0.9,zh;q=0.8,zh-CN;q=0.7"},
		{"Cookie", "BIDUPSID=EE7F8035EAF85C3FAA8C0ED90AE33BBB; PSTM=1592191861; BAIDUID=EE7F8035EAF85C3F702D40EC7B98EE2B:FG=1; BD_UPN=123353; BDORZ=B490B5EBF6F3CD402E515D22BCDA1598; COOKIE_SESSION=6154037_0_3_3_0_3_0_0_3_2_1_0_0_0_0_0_0_0_1599478984%7C3%230_0_1599478984%7C1"},
	};

	web::HttpHeader header(attrs);

	EXPECT_EQ(header.GetContentLength(), 0);
	EXPECT_EQ(std::string(header.GetConnection()), "keep-alive");
	EXPECT_EQ(std::string(header.GetUpgrade()), "");
	EXPECT_EQ(std::string(header.GetCookie("BIDUPSID")), "EE7F8035EAF85C3FAA8C0ED90AE33BBB");
	EXPECT_EQ(std::string(header.GetCookie("PSTM")), "1592191861");
	EXPECT_EQ(std::string(header.GetCookie("BAIDUID")), "EE7F8035EAF85C3F702D40EC7B98EE2B:FG=1");
	EXPECT_EQ(std::string(header.GetCookie("BD_UPN")), "123353");
	EXPECT_EQ(std::string(header.GetCookie("BDORZ")), "B490B5EBF6F3CD402E515D22BCDA1598");
	EXPECT_EQ(std::string(header.GetCookie("COOKIE_SESSION")), "6154037_0_3_3_0_3_0_0_3_2_1_0_0_0_0_0_0_0_1599478984%7C3%230_0_1599478984%7C1");
	EXPECT_EQ(header.GetHttpAttrs().size(), 14);
}
