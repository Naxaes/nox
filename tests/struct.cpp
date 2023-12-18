#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lib.h"
}



TEST(StructTest, SimpleStruct) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo {}");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneField) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a : int }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructTwoFields) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a : int b : int }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneFieldsWithInit) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a : int = 0 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructTwoFieldsWithInit) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a : int = 0 b : int = 0 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneFieldAndUsage) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a : int } foo := Foo { a = 69 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructTwoFieldsAndUsage) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a : int b : int } foo := Foo { a = 69 b = 420 }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneFieldWithInitAndUsageAndAccess) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a : int } foo := Foo { a = 69 } foo.a");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(StructTest, SimpleStructTwoFieldWithInitAndUsageAndAccess1) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a: int b: int } foo := Foo { a = 69 b = 420 } foo.a");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(StructTest, SimpleStructTwoFieldWithInitAndUsageAndAccess2) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a: int b: int } foo := Foo { a = 69 b = 420 } foo.b");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(StructTest, SimpleStructTwoFieldWithInitAndUsageAndAccess3) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("struct Foo { a: int b: int } foo := Foo { a = 35 b = 34 } foo.a + foo.b");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}
