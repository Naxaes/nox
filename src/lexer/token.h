#pragma once

typedef enum {
    TokenGroup_None = 0,
    TokenGroup_Literal = 1 << 0,
    TokenGroup_Binary_Arithmetic_Operator = 1 << 1,
    TokenGroup_Binary_Comparison_Operator = 1 << 2,
    TokenGroup_Binary_Logical_Operator = 1 << 3,
    TokenGroup_Keyword = 1 << 4,
} TokenGroup;


// X(upper, lower, repr, group)
#define ALL_TOKENS(X) \
    X(Number,         number,        0,         Literal)                            \
    X(Real,           real,          0,         Literal)                            \
    X(String,         string,        0,         Literal)                            \
    X(Identifier,     identifier,    0,         None)                               \
    X(Plus,           plus,          "+",       Binary_Arithmetic_Operator)         \
    X(Minus,          minus,         "-",       Binary_Arithmetic_Operator)         \
    X(Asterisk,       asterisk,      "*",       Binary_Arithmetic_Operator)         \
    X(Slash,          slash,         "/",       Binary_Arithmetic_Operator)         \
    X(Percent,        percent,       "%",       Binary_Arithmetic_Operator)         \
    X(Less,           less,          "<",       Binary_Comparison_Operator)         \
    X(Less_Equal,     less_equal,    "<=",      Binary_Comparison_Operator)         \
    X(Equal_Equal,    equal_equal,   "==",      Binary_Comparison_Operator)         \
    X(Bang_Equal,     bang_equal,    "!=",      Binary_Comparison_Operator)         \
    X(Greater_Equal,  greater_equal, ">=",      Binary_Comparison_Operator)         \
    X(Greater,        greater,       ">",       Binary_Comparison_Operator)         \
    X(Bang,           bang,          "!",       None)                               \
    X(Equal,          equal,         "=",       None)                               \
    X(Colon,          colon,         ":",       None)                               \
    X(Colon_Equal,    colon_equal,   ":=",      None)                               \
    X(True,           true,          "true",    Keyword)                            \
    X(False,          false,         "false",   Keyword)                            \
    X(Not,            not,           "not",     Keyword|TokenGroup_Binary_Logical_Operator)                            \
    X(And,            and,           "and",     Keyword|TokenGroup_Binary_Logical_Operator)                            \
    X(Or,             or,            "or",      Keyword|TokenGroup_Binary_Logical_Operator)                            \
    X(If,             if,            "if",      Keyword)                            \
    X(Else,           else,          "else",    Keyword)                            \
    X(Then,           then,          "then",    Keyword)                            \
    X(While,          while,         "while",   Keyword)                            \
    X(Fun,            fun,           "fun",     Keyword)                            \
    X(Return,         return,        "return",  Keyword)                            \
    X(Open_Paren,     open_paren,    "(",       None)                               \
    X(Close_Paren,    close_paren,   ")",       None)                               \
    X(Open_Brace,     open_brace,    "{",       None)                               \
    X(Close_Brace,    close_brace,   "}",       None)                               \
    X(Comma,          comma,         ",",       None)                               \
    X(Eof,            eof,           "\0",      None)


typedef enum {
#define X(upper, lower, body, value) Token_##upper,
    ALL_TOKENS(X)
    TOKEN_LAST = Token_Eof,
#undef X
} Token;


const char* token_repr(Token token);
TokenGroup token_group(Token token);

