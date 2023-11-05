#include <gtest/gtest.h>
#include "gmock/gmock.h"

using testing::ElementsAre;


#include <vector>

extern "C" {
#include "lexer/lexer.h"
}

static const char* source = "34 + 34 + 1";


TEST(LexerTest, TokenizationTest) {
    // Prepare test input
    TokenArray token_array = lexer_lex(source);

    std::vector<Token> tokens(token_array.tokens, token_array.tokens+token_array.size);
    EXPECT_THAT(tokens, testing::ElementsAre(
            Token_Number,
            Token_Plus,
            Token_Number,
            Token_Plus,
            Token_Number,
            Token_Eof
    ));

    std::vector<std::string> reprs;
    reprs.reserve(token_array.size);
    for (size_t i = 0; i < token_array.size; ++i) {
        reprs.emplace_back(lexer_repr_of(token_array, i));
    }

    std::string expected_strings[] = {
            "34",
            "+",
            "34",
            "+",
            "1",
            "<EOF>"
    };
    EXPECT_THAT(reprs, testing::ElementsAreArray(expected_strings));
}
