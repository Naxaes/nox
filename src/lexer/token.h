#ifndef TOKEN_H
#define TOKEN_H

#include "preamble.h"

typedef enum {
    TokenGroup_None = 0,
    TokenGroup_Literal,
    TokenGroup_Binary_Arithmetic_Operator,
    TokenGroup_Binary_Comparison_Operator,
    TokenGroup_Keyword,
} TokenGroup;


typedef enum {
    Token_Number,
    Token_Real,
    Token_String,
    Token_Identifier,
    Token_Plus,
    Token_Minus,
    Token_Asterisk,
    Token_Slash,
    Token_Percent,
    Token_Less,
    Token_Less_Equal,
    Token_Equal_Equal,
    Token_Bang_Equal,
    Token_Greater_Equal,
    Token_Greater,
    Token_Bang,
    Token_Equal,
    Token_Colon,
    Token_Colon_Equal,
    Token_If,
    Token_Else,
    Token_While,
    Token_Fun,
    Token_Return,
    Token_Open_Paren,
    Token_Close_Paren,
    Token_Open_Brace,
    Token_Close_Brace,
    Token_Comma,
    Token_Eof,
} Token;
#define TOKEN_COUNT 30

static const Token ALL_TOKENS[TOKEN_COUNT];
const char* token_repr(Token token);
TokenGroup token_group(Token token);

#endif  // TOKEN_H


#ifdef TOKEN_IMPLEMENTATION
STATIC_ASSERT(TOKEN_COUNT == 30, Token_count_has_changed);
static const Token ALL_TOKENS[TOKEN_COUNT] = {
        Token_Number,
        Token_Real,
        Token_String,
        Token_Identifier,
        Token_Plus,
        Token_Minus,
        Token_Asterisk,
        Token_Slash,
        Token_Percent,
        Token_Less,
        Token_Less_Equal,
        Token_Equal_Equal,
        Token_Bang_Equal,
        Token_Greater_Equal,
        Token_Greater,
        Token_Bang,
        Token_Equal,
        Token_Colon,
        Token_Colon_Equal,
        Token_If,
        Token_Else,
        Token_While,
        Token_Fun,
        Token_Return,
        Token_Open_Paren,
        Token_Close_Paren,
        Token_Open_Brace,
        Token_Close_Brace,
        Token_Comma,
        Token_Eof,
};

const char* token_repr(Token token) {
    STATIC_ASSERT(TOKEN_COUNT == 30, Token_count_has_changed);
    switch (token) {
        case Token_Number:
            return 0;
        case Token_Real:
            return 0;
        case Token_String:
            return 0;
        case Token_Identifier:
            return 0;
        case Token_Plus:
            return "+";
        case Token_Minus:
            return "-";
        case Token_Asterisk:
            return "*";
        case Token_Slash:
            return "/";
        case Token_Percent:
            return "%";
        case Token_Less:
            return "<";
        case Token_Less_Equal:
            return "<=";
        case Token_Equal_Equal:
            return "==";
        case Token_Bang_Equal:
            return "!=";
        case Token_Greater_Equal:
            return ">=";
        case Token_Greater:
            return ">";
        case Token_Bang:
            return "!";
        case Token_Equal:
            return "=";
        case Token_Colon:
            return ":";
        case Token_Colon_Equal:
            return ":=";
        case Token_If:
            return "if";
        case Token_Else:
            return "else";
        case Token_While:
            return "while";
        case Token_Fun:
            return "fun";
        case Token_Return:
            return "return";
        case Token_Open_Paren:
            return "(";
        case Token_Close_Paren:
            return ")";
        case Token_Open_Brace:
            return "{";
        case Token_Close_Brace:
            return "}";
        case Token_Comma:
            return ",";
        case Token_Eof:
            return "\0";
    }
}


TokenGroup token_group(Token token) {
    STATIC_ASSERT(TOKEN_COUNT == 30, Token_count_has_changed);
    switch (token) {
        case Token_Number:
            return TokenGroup_Literal;
        case Token_Real:
            return TokenGroup_Literal;
        case Token_String:
            return TokenGroup_Literal;
        case Token_Identifier:
            return TokenGroup_None;
        case Token_Plus:
            return TokenGroup_Binary_Arithmetic_Operator;
        case Token_Minus:
            return TokenGroup_Binary_Arithmetic_Operator;
        case Token_Asterisk:
            return TokenGroup_Binary_Arithmetic_Operator;
        case Token_Slash:
            return TokenGroup_Binary_Arithmetic_Operator;
        case Token_Percent:
            return TokenGroup_Binary_Arithmetic_Operator;
        case Token_Less:
            return TokenGroup_Binary_Comparison_Operator;
        case Token_Less_Equal:
            return TokenGroup_Binary_Comparison_Operator;
        case Token_Equal_Equal:
            return TokenGroup_Binary_Comparison_Operator;
        case Token_Bang_Equal:
            return TokenGroup_Binary_Comparison_Operator;
        case Token_Greater_Equal:
            return TokenGroup_Binary_Comparison_Operator;
        case Token_Greater:
            return TokenGroup_Binary_Comparison_Operator;
        case Token_Bang:
            return TokenGroup_None;
        case Token_Equal:
            return TokenGroup_None;
        case Token_Colon:
            return TokenGroup_None;
        case Token_Colon_Equal:
            return TokenGroup_None;
        case Token_If:
            return TokenGroup_Keyword;
        case Token_Else:
            return TokenGroup_Keyword;
        case Token_While:
            return TokenGroup_Keyword;
        case Token_Fun:
            return TokenGroup_Keyword;
        case Token_Return:
            return TokenGroup_Keyword;
        case Token_Open_Paren:
            return TokenGroup_None;
        case Token_Close_Paren:
            return TokenGroup_None;
        case Token_Open_Brace:
            return TokenGroup_None;
        case Token_Close_Brace:
            return TokenGroup_None;
        case Token_Comma:
            return TokenGroup_None;
        case Token_Eof:
            return TokenGroup_None;
    }
}
#endif  // TOKEN_IMPLEMENTATION

