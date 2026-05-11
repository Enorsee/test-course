#include <gtest/gtest.h>
#include <string>

extern "C" {
#include "../cJSON/cJSON.h"
}

// Unit test #1
TEST(CJSONTest, ParseSimpleObject) {
    const char *json = "{\"name\":\"Alice\",\"age\":18}";
    cJSON *root = cJSON_Parse(json);

    ASSERT_NE(root, nullptr);

    cJSON *name = cJSON_GetObjectItem(root, "name");
    cJSON *age = cJSON_GetObjectItem(root, "age");

    ASSERT_NE(name, nullptr);
    ASSERT_NE(age, nullptr);

    EXPECT_TRUE(cJSON_IsString(name));
    EXPECT_TRUE(cJSON_IsNumber(age));
    EXPECT_STREQ(name->valuestring, "Alice");
    EXPECT_EQ(age->valueint, 18);

    cJSON_Delete(root);
}

// *******************************
// ***  Write your tests here  ***
// *******************************

// Unit test #2



// Unit test #3



// ... ...
