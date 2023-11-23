#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lib.h"
}



TEST(FunctionDeclTest, Simple) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() {} main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 0);
}

TEST(FunctionDeclTest, SimpleWithReturn) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() { return 69 } main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithReturnAndAdd) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() { return 34 + 35 } main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}


TEST(FunctionDeclTest, SimpleWithUntypedParameter) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main(a) { return a } main(69)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithMultipleUntypedParameter) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main(a, b) { return a + b } main(34, 35)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithTypedParameter) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main(a: int) { return a } main(69)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithTypedParameterAndAdd) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main(a: int) { return a + 35 } main(34)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithTypedReturn) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() int { return 69 } main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithTypedReturnAndAdd) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() int { return 34 + 35 } main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithTypedReturnAndTypedParameter) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main(a: int) int { return a + 35 } main(34)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithUnTypedReturnAndMultipleTypedParameters) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main(a: int, b: int) { return a + b } main(34, 35)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, SimpleWithTypedReturnAndMultipleTypedParameters) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main(a: int, b: int) int { return a + b } main(34, 35)");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, Nested) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() int { fun nested() int { return 69 } return nested() } main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result, 69);
}

TEST(FunctionDeclTest, NestedWithParameter) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() int { fun nested(a: int) int { return a } return nested(69) } main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result,69);
}

TEST(FunctionDeclTest, NestedWithMultipleParameters) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR("fun main() int { fun nested(a: int, b: int) int { return a + b } return nested(34, 35) } main()");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result,69);
}

TEST(FunctionDeclTest, DeferredFunDecl) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    Str source = STR(" main() fun main() int { return nested(69) } fun nested(a: int) int { return a }");

    InterpreterResult result = run_from_source(STR("<test>"), source, &logger);
    ASSERT_EQ(result.error,  0);
    ASSERT_EQ(result.result,69);
}