#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lib.h"
}



TEST(LogicTest, True) {
    Str source = STR("true");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, False) {
    Str source = STR("false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Not1) {
    Str source = STR("not true");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Not2) {
    Str source = STR("not false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, And1) {
    Str source = STR("true and true");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, And2) {
    Str source = STR("true and false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, And3) {
    Str source = STR("false and true");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, And4) {
    Str source = STR("false and false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Or1) {
    Str source = STR("true or true");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Or2) {
    Str source = STR("true or false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Or3) {
    Str source = STR("false or true");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Or4) {
    Str source = STR("false or false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Precedence1) {
    Str source = STR("true and true or false and false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Precedence2) {
    Str source = STR("true and (true or false) and false");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Precedence3) {
    Str source = STR("(true and true) or (false and false)");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}
