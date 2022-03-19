#include <gtest/gtest.h>
#include "JsonObjTest.cpp"
#include "HttpAttrTest.cpp"
#include "HttpHeaderTest.cpp"
#include "HttpRequestTest.cpp"
#include "HttpResponseTest.cpp"
#include "MysqlServiceTest.cpp"
#include "WareHouseServiceTest.cpp"
#include "MaterialServiceTest.cpp"
#include "ItemInventoryServiceTest.cpp"
#include "CheckServiceTest.cpp"

int main(int _argc, char** _argv)
{
        ::testing::InitGoogleTest(&_argc, _argv);

        return RUN_ALL_TESTS();
}

