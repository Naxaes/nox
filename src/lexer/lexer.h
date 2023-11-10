#pragma once
#include "types.h"
#include "str.h"
#include "token.h"

typedef u32 TokenId;
typedef u32 IdentId;

typedef struct {
    Str name;
    Str source;

    Token*   tokens;
    IdentId* identifiers;
    size_t*  indices;
    TokenId  size;

    u8*    interned_strings;
    size_t interned_string_size;
} TokenArray;


/// Lex the source to a token array.
TokenArray lexer_lex(Str name, Str source);

/// Get the textual representation of a token.
const char* lexer_repr_of(TokenArray tokens, TokenId id);

/// Free the memory allocated by the token array.
void token_array_free(TokenArray tokens);

