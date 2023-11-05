#pragma once
#include "types.h"
#include "str.h"

typedef u32 TokenId;
typedef u32 IdentId;

typedef enum {
    Token_Invalid,
    Token_Number,
    Token_Plus,
    Token_Eof,
} Token;
#define TOKEN_COUNT (Token_Eof + 1)

typedef struct {
    Token*   tokens;
    IdentId* identifiers;
    size_t   size;

    u32*   intern_string_lookup;
    u8*    interned_strings;
    size_t interned_string_size;
} TokenArray;


/// Lex the source to a token array.
TokenArray lexer_lex(const char* source);

/// Get the textual representation of a token.
const char* lexer_repr_of(TokenArray tokens, TokenId id);



