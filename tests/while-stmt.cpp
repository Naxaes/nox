#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lib.h"
}



TEST(WhileStmtTest, SimpleWhile) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("a := 0 while a < 69 { a = a + 1 } a");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(WhileStmtTest, SimpleWhile2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("a := 0 while a < 69 { print(\"loop\") a = a + 1 } a");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}
