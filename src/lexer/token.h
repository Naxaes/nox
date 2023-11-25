#pragma once

typedef enum {
    TokenGroup_None = 0,
    TokenGroup_Literal = 1 << 0,
    TokenGroup_Arithmetic_Operator = 1 << 1,
    TokenGroup_Comparison_Operator = 1 << 2,
    TokenGroup_Logical_Operator = 1 << 3,
    TokenGroup_Keyword = 1 << 4,
} TokenGroup;


// X(upper, lower, repr, group)
#define ALL_TOKENS(X) \
    X(Number,         number,        0,         TokenGroup_Literal)                            \
    X(Real,           real,          0,         TokenGroup_Literal)                            \
    X(String,         string,        0,         TokenGroup_Literal)                            \
    X(Identifier,     identifier,    0,         TokenGroup_None)                               \
    X(Plus,           plus,          "+",       TokenGroup_Arithmetic_Operator)                \
    X(Minus,          minus,         "-",       TokenGroup_Arithmetic_Operator)                \
    X(Asterisk,       asterisk,      "*",       TokenGroup_Arithmetic_Operator)                \
    X(Slash,          slash,         "/",       TokenGroup_Arithmetic_Operator)                \
    X(Percent,        percent,       "%",       TokenGroup_Arithmetic_Operator)                \
    X(Less,           less,          "<",       TokenGroup_Comparison_Operator)                \
    X(Less_Equal,     less_equal,    "<=",      TokenGroup_Comparison_Operator)                \
    X(Equal_Equal,    equal_equal,   "==",      TokenGroup_Comparison_Operator)                \
    X(Bang_Equal,     bang_equal,    "!=",      TokenGroup_Comparison_Operator)                \
    X(Greater_Equal,  greater_equal, ">=",      TokenGroup_Comparison_Operator)                \
    X(Greater,        greater,       ">",       TokenGroup_Comparison_Operator)                \
    X(Bang,           bang,          "!",       TokenGroup_Comparison_Operator)                \
    X(Equal,          equal,         "=",       TokenGroup_None)                               \
    X(Colon,          colon,         ":",       TokenGroup_None)                               \
    X(Colon_Equal,    colon_equal,   ":=",      TokenGroup_None)                               \
    X(Dot,            dot,           ".",       TokenGroup_None)                               \
    X(True,           true,          "true",    TokenGroup_Keyword|TokenGroup_Literal)                            \
    X(False,          false,         "false",   TokenGroup_Keyword|TokenGroup_Literal)                            \
    X(Not,            not,           "not",     TokenGroup_Keyword|TokenGroup_Logical_Operator) \
    X(And,            and,           "and",     TokenGroup_Keyword|TokenGroup_Logical_Operator) \
    X(Or,             or,            "or",      TokenGroup_Keyword|TokenGroup_Logical_Operator) \
    X(If,             if,            "if",      TokenGroup_Keyword)                            \
    X(Else,           else,          "else",    TokenGroup_Keyword)                            \
    X(Then,           then,          "then",    TokenGroup_Keyword)                            \
    X(While,          while,         "while",   TokenGroup_Keyword)                            \
    X(Fun,            fun,           "fun",     TokenGroup_Keyword)                            \
    X(Return,         return,        "return",  TokenGroup_Keyword)                            \
    X(Struct,         struct,        "struct",  TokenGroup_Keyword)                            \
    X(Open_Paren,     open_paren,    "(",       TokenGroup_None)                               \
    X(Close_Paren,    close_paren,   ")",       TokenGroup_None)                               \
    X(Open_Brace,     open_brace,    "{",       TokenGroup_None)                               \
    X(Close_Brace,    close_brace,   "}",       TokenGroup_None)                               \
    X(Comma,          comma,         ",",       TokenGroup_None)                               \
    X(Eof,            eof,           "\0",      TokenGroup_None)


typedef enum {
#define X(upper, lower, body, value) Token_##upper,
    ALL_TOKENS(X)
    TOKEN_LAST = Token_Eof,
#undef X
} Token;


const char* token_repr(Token token);
TokenGroup token_group(Token token);

