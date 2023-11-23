#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lib.h"
}



TEST(ArithmeticTest, Integer) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Add) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 + 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 6);
}

TEST(ArithmeticTest, Sub) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 - 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, -4);
}

TEST(ArithmeticTest, Mul) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("2 * 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 10);
}

TEST(ArithmeticTest, Div) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("10 / 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 2);
}

TEST(ArithmeticTest, Mod) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("10 % 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Lt) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 < 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Gt) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 > 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Lte) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 <= 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Gte) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 >= 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Eq) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 == 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Ne) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 != 5");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Parentheses) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("(1 + 5) * 2");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 12);
}

TEST(ArithmeticTest, Precedence) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 + 5 * 2");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 11);
}

TEST(ArithmeticTest, Precedence2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 * 5 + 2");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 7);
}

TEST(ArithmeticTest, Precedence3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("1 + 5 * 2 + 3 * 4 + 6 * 7 + 8 * 9 + 10");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 147);
}




