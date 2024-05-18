#pragma once

#include "preamble.h"
#include "str.h"
#include "token.h"
#include "logger.h"

typedef u32 TokenIndex;
typedef u32 DataPoolIndex;
typedef u32 SourceIndex;

typedef struct {
    Str name;
    Str source;

    /// Parallel arrays.
    /// - tokens: The token kinds.
    /// - identifiers: Id's to the data_pool.
    /// - source_offsets: The offset in the source where the token starts.
    Token*          tokens;
    DataPoolIndex*  identifiers;
    SourceIndex*    source_offsets;
    TokenIndex      size;

    u8*    data_pool;
    size_t data_pool_size;
} TokenArray;


/// Lex the source to a token array.
TokenArray lexer_lex(Str name, Str source, Logger* logger);

/// Get the textual representation of a token.
const char* lexer_repr_of(TokenArray tokens, TokenIndex id);

/// Free the memory allocated by the token array.
void token_array_free(TokenArray tokens);

