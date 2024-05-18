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
    Str source = STR("struct Foo {}");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneField) {
    Str source = STR("struct Foo { a : int }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructTwoFields) {
    Str source = STR("struct Foo { a : int b : int }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneFieldsWithInit) {
    Str source = STR("struct Foo { a : int = 0 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructTwoFieldsWithInit) {
    Str source = STR("struct Foo { a : int = 0 b : int = 0 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneFieldAndUsage) {
    Str source = STR("struct Foo { a : int } foo := Foo { a = 69 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructTwoFieldsAndUsage) {
    Str source = STR("struct Foo { a : int b : int } foo := Foo { a = 69 b = 420 }");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(StructTest, SimpleStructOneFieldWithInitAndUsageAndAccess) {
    Str source = STR("struct Foo { a : int } foo := Foo { a = 69 } foo.a");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(StructTest, SimpleStructTwoFieldWithInitAndUsageAndAccess1) {
    Str source = STR("struct Foo { a: int b: int } foo := Foo { a = 69 b = 420 } foo.a");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(StructTest, SimpleStructTwoFieldWithInitAndUsageAndAccess2) {
    Str source = STR("struct Foo { a: int b: int } foo := Foo { a = 69 b = 420 } foo.b");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 420);
}

TEST(StructTest, SimpleStructTwoFieldWithInitAndUsageAndAccess3) {
    Str source = STR("struct Foo { a: int b: int } foo := Foo { a = 35 b = 34 } foo.a + foo.b");

    InterpreterResult result = run(STR("<test>"), source, 0);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}
