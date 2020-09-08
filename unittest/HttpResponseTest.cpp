#include <gtest/gtest.h>
#include "../src/web.h"

TEST(HttpResponse, Vaild)
{
	const char body[] = "a body";

	web::HttpResponse response(200, {}, body, sizeof(body));

	EXPECT_EQ(response.GetStateCode(), 200);
	EXPECT_EQ(response.GetBodySize(), 7);
}
