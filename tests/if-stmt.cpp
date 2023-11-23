#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lib.h"
}



TEST(IfStmtTest, SimpleIfWithThen) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 1 then 69");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithThenAndElse1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 1 then 69 else 420");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}


TEST(IfStmtTest, SimpleIfWithThenAndElse2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 then 69 else 420");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}


TEST(IfStmtTest, SimpleIfWithThenAndElseIfAndElse1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 1 then 69 else if 2 == 2 then 420 else 1337");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithThenAndElseIfAndElse2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 then 69 else if 2 == 2 then 420 else 1337");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(IfStmtTest, SimpleIfWithThenAndElseIfAndElse3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 then 69 else if 2 == 3 then 420 else 1337");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1337);
}

TEST(IfStmtTest, SimpleIfWithBlock) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 1 { 69 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElse1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 1 { 69 } else { 420 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElse2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 { 69 } else { 420 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIf1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 1 { 69 } else if 2 == 2 { 420 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIf2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 { 69 } else if 2 == 2 { 420 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIf3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 { 69 } else if 2 == 2 { 420 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIfAndElse1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 1 { 69 } else if 2 == 2 { 420 } else { 1337 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIfAndElse2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 { 69 } else if 2 == 2 { 420 } else { 1337 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIfAndElse3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("if 1 == 2 { 69 } else if 2 == 3 { 420 } else { 1337 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1337);
}
