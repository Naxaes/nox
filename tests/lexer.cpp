#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;

#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lexer/lexer.h"
}


void test_tokenization(Str source, std::vector<Token> expected_tokens, std::vector<std::string> expected_reprs) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_ERROR, stderr);
    TokenArray token_array = lexer_lex(STR("<test>"), source, &logger);

    expected_tokens.push_back(Token_Eof);
    expected_reprs.emplace_back("");

    EXPECT_EQ(token_array.size, expected_tokens.size());
    size_t size = std::min((size_t)token_array.size, expected_tokens.size());

    std::vector<Token> tokens(token_array.tokens, token_array.tokens+token_array.size);
    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(token_array.tokens[i], expected_tokens[i]);
    }

    std::vector<std::string> reprs;
    reprs.reserve(token_array.size);
    for (size_t i = 0; i < token_array.size; ++i) {
        reprs.emplace_back(lexer_repr_of(token_array, i));
    }

    EXPECT_EQ(reprs.size(), expected_reprs.size());
    size = std::min(reprs.size(), expected_reprs.size());
    for (size_t i = 0; i < size; ++i) {
        EXPECT_EQ(reprs[i], expected_reprs[i]);
    }
}

TEST(LexerTest, Empty) {
    test_tokenization(STR(""), {}, {});
}

TEST(LexerTest, ValidNumber) {
    test_tokenization(STR("0"), { Token_Number }, { "0" });
    test_tokenization(STR("123"), { Token_Number, }, { "123" });
}

TEST(LexerTest, ValidReal) {
    test_tokenization(STR("0.0"), { Token_Real }, { "0.0" });
    test_tokenization(STR("1.02"), { Token_Real }, { "1.02" });
    test_tokenization(STR("0.01"), { Token_Real }, { "0.01" });
    test_tokenization(STR("123.123"), { Token_Real }, { "123.123" });
}

TEST(LexerTest, ValidString) {
    test_tokenization(STR("\"\""), { Token_String }, { "" });
    test_tokenization(STR("\"a\""), { Token_String }, { "a" });
    test_tokenization(STR("\"abc\""), { Token_String }, { "abc" });
    test_tokenization(STR("\"a b c\""), { Token_String }, { "a b c" });
    test_tokenization(STR("\"a\nb c\""), { Token_String }, { "a\nb c" });
    test_tokenization(STR(R"("a\nb\tc")"), { Token_String }, { R"(a\nb\tc)" });
    test_tokenization(STR(R"("a\"b c")"), { Token_String }, { R"(a\"b c)" });
}

TEST(LexerTest, ValidIdentifier) {
    test_tokenization(STR("a"), { Token_Identifier }, { "a" });
    test_tokenization(STR("abc"), { Token_Identifier }, { "abc" });
    test_tokenization(STR("a_b_c"), { Token_Identifier }, { "a_b_c" });
    test_tokenization(STR("a0"), { Token_Identifier }, { "a0" });
    test_tokenization(STR("a0b"), { Token_Identifier }, { "a0b" });
    test_tokenization(STR("a0_b1_c2"), { Token_Identifier }, { "a0_b1_c2" });
}

TEST(LexerTest, Keywords) {
    test_tokenization(STR("if"), { Token_If }, { "if" });
    test_tokenization(STR("else"), { Token_Else }, { "else" });
    test_tokenization(STR("while"), { Token_While }, { "while" });
}

TEST(LexerTest, BinaryOperators) {
    test_tokenization(STR("+"), { Token_Plus }, { "+" });
    test_tokenization(STR("-"), { Token_Minus }, { "-" });
    test_tokenization(STR("*"), { Token_Asterisk }, { "*" });
    test_tokenization(STR("/"), { Token_Slash }, { "/" });
    test_tokenization(STR("%"), { Token_Percent }, { "%" });

    test_tokenization(STR("<"), { Token_Less }, { "<" });
    test_tokenization(STR("<="), { Token_Less_Equal }, { "<=" });
    test_tokenization(STR("=="), { Token_Equal_Equal }, { "==" });
    test_tokenization(STR("!="), { Token_Bang_Equal }, { "!=" });
    test_tokenization(STR(">="), { Token_Greater_Equal }, { ">=" });
    test_tokenization(STR(">"), { Token_Greater }, { ">" });
}

TEST(LexerTest, Assignment) {
    test_tokenization(STR("="), { Token_Equal }, { "=" });
    test_tokenization(STR(":="), { Token_Colon_Equal }, { ":=" });
}

TEST(LexerTest, Parentheses) {
    test_tokenization(STR("("), { Token_Open_Paren }, { "(" });
    test_tokenization(STR(")"), { Token_Close_Paren }, { ")" });
    test_tokenization(STR("{"), { Token_Open_Brace }, { "{" });
    test_tokenization(STR("}"), { Token_Close_Brace }, { "}" });
}

TEST(LexerTest, Comma) {
    test_tokenization(STR(","), { Token_Comma }, { "," });
}

TEST(LexerTest, Whitespace) {
    test_tokenization(STR(" "), {}, {});
    test_tokenization(STR("\t"), {}, {});
    test_tokenization(STR("\n"), {}, {});
    test_tokenization(STR("\r"), {}, {});
    test_tokenization(STR("\r\n"), {}, {});
    test_tokenization(STR(" \t\n\r"), {}, {});
}

TEST(LexerTest, Comments) {
    test_tokenization(STR("//\n"), {}, {});
    test_tokenization(STR("//hello\n"), {}, {});
    test_tokenization(STR("//\r\n"), {}, {});
    test_tokenization(STR("// testing "), {}, {});

    test_tokenization(STR("/**/"), {}, {});
    test_tokenization(STR("/*hello*/"), {}, {});
    test_tokenization(STR("/*hello\n*/"), {}, {});
    test_tokenization(STR("/*\n\n*/"), {}, {});
    test_tokenization(STR("/* hello\n /* hello*/ \n*/"), {}, {});
}

//TEST(LexerTest, AllTokens) {
//    test_tokenization(
//            STR("0 123 0.0 1.02 0.01 123.123 \"\" \"a\" \"abc\" \"a b c\" \"a\nb c\" \"a\\nb\\tc\" \"a\\\"b c\" a abc a_b_c a0 a0b a0_b1_c2 if else while + - * / % < <= == != >= > = := : . true false not and or ( ) { } ,"),
//            {Token_Number, Token_Number, Token_Real, Token_Real, Token_Real, Token_Real, Token_String, Token_String,
//             Token_String, Token_String, Token_String, Token_String, Token_String, Token_Identifier, Token_Identifier, Token_Identifier, Token_Identifier,
//             Token_Identifier, Token_Identifier, Token_If, Token_Else, Token_While, Token_Plus, Token_Minus,
//             Token_Asterisk, Token_Slash, Token_Percent, Token_Less, Token_Less_Equal, Token_Equal_Equal,
//             Token_Bang_Equal, Token_Greater_Equal, Token_Greater, Token_Equal, Token_Colon_Equal, Token_Colon,
//             Token_Dot, Token_True, Token_False, Token_Not, Token_And, Token_Or, Token_Open_Paren, Token_Close_Paren,
//             Token_Open_Brace, Token_Close_Brace, Token_Comma},
//            {"0", "123", "0.0", "1.02", "0.01", "123.123", "", "a", "abc", "a b c", "a\nb c", "a\\nb\\tc", "a\\\"b c",
//             "a", "abc", "a_b_c", "a0", "a0b", "a0_b1_c2", "if", "else", "while", "+", "-", "*", "/", "%", "<", "<=", "==", "!=", ">=",
//             ">", "=", ":=", ":", ".", "true", "false", "not", "and", "or", "(", ")", "{", "}", ","}
//    );
//}
//

