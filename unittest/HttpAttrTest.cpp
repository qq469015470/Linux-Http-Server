#include <gtest/gtest.h>
#include "../src/web.h"

TEST(HttpAttr, GetSetTest)
{
	web::HttpAttr temp("Content-Length", "1104");

	EXPECT_STREQ(temp.GetKey().c_str(), "content-length");
	EXPECT_STREQ(temp.GetValue().c_str(), "1104");
}
