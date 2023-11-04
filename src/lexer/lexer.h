#pragma once
#include "types.h"
#include "str.h"


typedef u32 TokenId;

typedef enum {
    Token_Invalid,
    Token_Literal,
    Token_Eof,
} Token;

typedef struct {
    Token* tokens;
    Str*   representations;
    size_t size;
} TokenArray;


/// Lex the source to a token array.
TokenArray lexer_lex(const char* source);

/// Get the textual representation of a token.
Str lexer_repr_of(TokenArray tokens, TokenId id);



