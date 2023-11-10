#pragma once

typedef enum {
    Token_Invalid = 0,

    Token_Number,
        Token_Literal_Start = Token_Number,
        Token_Real,
        Token_String,
    Token_Literal_End = Token_String,

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
    Token_Colon,
    Token_Colon_Equal,

    Token_If,
        Token_Keywords_Start = Token_If,
        Token_Else,
        Token_Fun,
        Token_While,
    Token_Keywords_End = Token_While,

    Token_Open_Paren,
    Token_Close_Paren,
    Token_Open_Brace,
    Token_Close_Brace,

    Token_Comma,

    Token_Eof,
} Token;
#define TOKEN_COUNT (Token_Eof + 1)

static inline int token_is_literal(Token token) {
    return Token_Literal_Start <= token && token <= Token_Literal_End;
}

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



