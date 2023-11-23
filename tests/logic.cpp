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
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("true");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, False) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Not1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("not true");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Not2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("not false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, And1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("true and true");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, And2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("true and false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, And3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("false and true");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, And4) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("false and false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Or1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("true or true");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Or2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("true or false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Or3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("false or true");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Or4) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("false or false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Precedence1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("true and true or false and false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}

TEST(LogicTest, Precedence2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("true and (true or false) and false");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(LogicTest, Precedence3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("(true and true) or (false and false)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1);
}
