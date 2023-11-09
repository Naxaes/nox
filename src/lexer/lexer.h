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
    Token_Binary_Operator_Start,
        Token_Plus = Token_Binary_Operator_Start,
        Token_Asterisk,
    Token_Binary_Operator_End = Token_Asterisk,
    Token_Equal,
    Token_Eof,
} Token;
#define TOKEN_COUNT (Token_Eof + 1)

static inline int token_is_binary_operator(Token token) {
    return Token_Binary_Operator_Start <= token && token <= Token_Binary_Operator_End;
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



