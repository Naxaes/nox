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
    Str source = STR("1");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Add) {
    Str source = STR("1 + 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 6);
}

TEST(ArithmeticTest, Sub) {
    Str source = STR("1 - 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, -4);
}

TEST(ArithmeticTest, Mul) {
    Str source = STR("2 * 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 10);
}

TEST(ArithmeticTest, Div) {
    Str source = STR("10 / 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 2);
}

TEST(ArithmeticTest, Mod) {
    Str source = STR("10 % 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Lt) {
    Str source = STR("1 < 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Gt) {
    Str source = STR("1 > 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Lte) {
    Str source = STR("1 <= 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Gte) {
    Str source = STR("1 >= 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Eq) {
    Str source = STR("1 == 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 0);
}

TEST(ArithmeticTest, Ne) {
    Str source = STR("1 != 5");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 1);
}

TEST(ArithmeticTest, Parentheses) {
    Str source = STR("(1 + 5) * 2");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 12);
}

TEST(ArithmeticTest, Precedence) {
    Str source = STR("1 + 5 * 2");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 11);
}

TEST(ArithmeticTest, Precedence2) {
    Str source = STR("1 * 5 + 2");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 7);
}

TEST(ArithmeticTest, Precedence3) {
    Str source = STR("1 + 5 * 2 + 3 * 4 + 6 * 7 + 8 * 9 + 10");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error, 0);
    ASSERT_EQ(result.result, 147);
}
