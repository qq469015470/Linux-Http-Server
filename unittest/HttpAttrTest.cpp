#include <gtest/gtest.h>
#include "../src/web.h"

TEST(HttpAttr, GetSetTest)
{
	web::HttpAttr temp("Content-Length", "1104");

	EXPECT_EQ(temp.GetKey(), "Content-Length");
	EXPECT_EQ(temp.GetValue(), "1104");
}
