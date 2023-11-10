#pragma once
#include "types.h"
#include "str.h"

typedef u32 TokenId;
typedef u32 IdentId;

typedef enum {
    Token_Invalid,
    Token_Number,
    Token_Real,
    Token_Identifier,

    Token_Plus,
        Token_Binary_Operator_Start = Token_Plus,
        Token_Binary_Arithmetic_Operator_Start = Token_Plus,
            Token_Minus,
            Token_Asterisk,
            Token_Slash,
            Token_Percent,
        Token_Binary_Arithmetic_Operator_End = Token_Percent,

        Token_Less,
        Token_Binary_Comparison_Operator_Start = Token_Less,
            Token_Less_Equal,
            Token_Equal_Equal,
            Token_Exclamation_Equal,
            Token_Greater_Equal,
            Token_Greater,
        Token_Binary_Comparison_Operator_End = Token_Greater,
    Token_Binary_Operator_End = Token_Binary_Comparison_Operator_End,

    Token_Exclamation,
    Token_Equal,
    Token_Colon_Equal,

    Token_If,
    Token_Keywords_Start = Token_If,
    Token_Else,
    Token_Keywords_End = Token_Else,

    Token_Open_Brace,
    Token_Close_Brace,

    Token_Eof,
} Token;
#define TOKEN_COUNT (Token_Eof + 1)

static inline int token_is_binary_operator(Token token) {
    return Token_Binary_Operator_Start <= token && token <= Token_Binary_Operator_End;
}

static inline int token_is_binary_arithmetic_operator(Token token) {
    return Token_Binary_Arithmetic_Operator_Start <= token && token <= Token_Binary_Arithmetic_Operator_End;
}

static inline int token_is_binary_logic_operator(Token token) {
    return Token_Binary_Comparison_Operator_Start <= token && token <= Token_Binary_Comparison_Operator_End;
}

static inline int token_is_keyword(Token token) {
    return Token_Keywords_Start <= token && token <= Token_Keywords_End;
}

typedef struct {
    Str name;
    Str source;

    Token*   tokens;
    IdentId* identifiers;
    size_t*  indices;
    size_t   size;

    u8*    interned_strings;
    size_t interned_string_size;
} TokenArray;


/// Lex the source to a token array.
TokenArray lexer_lex(Str name, Str source);

/// Get the textual representation of a token.
const char* lexer_repr_of(TokenArray tokens, TokenId id);

void token_array_free(TokenArray tokens);

