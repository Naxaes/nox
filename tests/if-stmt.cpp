#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lib.h"
#include "logger.h"
}



TEST(IfStmtTest, SimpleIfWithThen) {
    Str source = STR("if 1 == 1 then 69");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithThenAndElse1) {
    Str source = STR("if 1 == 1 then 69 else 420");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}


TEST(IfStmtTest, SimpleIfWithThenAndElse2) {
    Str source = STR("if 1 == 2 then 69 else 420");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}


TEST(IfStmtTest, SimpleIfWithThenAndElseIfAndElse1) {
    Str source = STR("if 1 == 1 then 69 else if 2 == 2 then 420 else 1337");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithThenAndElseIfAndElse2) {
    Str source = STR("if 1 == 2 then 69 else if 2 == 2 then 420 else 1337");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(IfStmtTest, SimpleIfWithThenAndElseIfAndElse3) {
    Str source = STR("if 1 == 2 then 69 else if 2 == 3 then 420 else 1337");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1337);
}

TEST(IfStmtTest, SimpleIfWithBlock) {
    Str source = STR("if 1 == 1 { 69 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElse1) {
    Str source = STR("if 1 == 1 { 69 } else { 420 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElse2) {
    Str source = STR("if 1 == 2 { 69 } else { 420 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIf1) {
    Str source = STR("if 1 == 1 { 69 } else if 2 == 2 { 420 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIf2) {
    Str source = STR("if 1 == 2 { 69 } else if 2 == 2 { 420 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIfAndElse1) {
    Str source = STR("if 1 == 1 { 69 } else if 2 == 2 { 420 } else { 1337 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIfAndElse2) {
    Str source = STR("if 1 == 2 { 69 } else if 2 == 2 { 420 } else { 1337 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(IfStmtTest, SimpleIfWithBlockAndElseIfAndElse3) {
    Str source = STR("if 1 == 2 { 69 } else if 2 == 3 { 420 } else { 1337 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 1337);
}
