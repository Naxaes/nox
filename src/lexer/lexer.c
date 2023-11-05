#include "lexer.h"
#include "str.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    size_t size;
    char   data[];
} InlineStr;

typedef struct {
    size_t   count;
    Token*   tokens;
    IdentId* identifiers;
    Str*     representations;

    u32*   intern_string_lookup;
    u8*    interned_strings;
    size_t interned_string_size;
} Lexer;

IdentId intern_string(Lexer* lexer, Str string) {
    u64 index  = str_hash(string) & (1024-1);
    u32 offset = lexer->intern_string_lookup[index];

    do {
        // If the offset is 0, we have not found a match,
        // which means that the string is not interned yet.
        if (offset == 0) {
            offset = lexer->interned_string_size;

            // Copy over the data and a null pointer at the end.
            memcpy(lexer->interned_strings + lexer->interned_string_size, string.data, string.size);
            memset(lexer->interned_strings + lexer->interned_string_size + string.size, '\0', 1);

            lexer->interned_string_size += string.size + 1;
            return offset;
        }

        const char* candidate = (const char*) (lexer->interned_strings + offset);
        if (strncmp(string.data, candidate, 1024) == 0) {
            // We found a match! Returned the cached offset.
            return offset;
        } else {
            // Go to the next entry in the lookup table.
            index = (index + 1) & (1024-1);
            offset = lexer->intern_string_lookup[index];
        }
    } while (1);
}

const char* lexer_repr_of(TokenArray tokens, TokenId id) {
    const char* str = (const char*) &tokens.interned_strings[tokens.identifiers[id]];
    return str;
}


TokenArray lexer_lex(const char* source) {
    Lexer lexer =  {
        .count  = 0,
        .tokens = (Token*) malloc(1024),
        .identifiers = (IdentId*) malloc(1024 * sizeof(IdentId)),
        .representations = (Str*) malloc(1024 * sizeof(Str)),
        .intern_string_lookup = memset(malloc(1024 * sizeof(u32)), 0, 1024 * sizeof(u32)),
        .interned_strings     = (u8*) malloc(1024),
        .interned_string_size = sizeof(Str),  // Skip the first few bytes so that 0 from the lookup table means "not found".
    };

    while (1) {
        // Skip whitespace.
        while (*source == ' ' || *source == '\t' || *source == '\n' || *source == '\r') {
            ++source;
        }

        switch (*source) {
            case '\0': {
                lexer.tokens[lexer.count++] = (Token) { Token_Eof };
                free(lexer.intern_string_lookup);
                return (TokenArray) {
                    lexer.tokens,
                    lexer.identifiers,
                    lexer.count,
                    lexer.intern_string_lookup,
                    lexer.interned_strings,
                    lexer.interned_string_size
                };
            } break;
            case '+': {
                lexer.tokens[lexer.count++] = (Token) { Token_Plus };
                ++source;
            } break;
            default: {
                if ('0' <= *source && *source <= '9') {
                    const char* start = source;
                    do {
                        ++source;
                    } while ('0' <= *source && *source <= '9');
                    const char* end = source;


                    Str string = (Str) { (ssize_t)(end-start), start };
                    lexer.representations[lexer.count] = string;

                    IdentId ident = intern_string(&lexer, string);
                    lexer.identifiers[lexer.count] = ident;

                    lexer.tokens[lexer.count++] = (Token) {Token_Number };
                    break;
                }

                fprintf(stderr, "Unknown token: '%c'\n", *source);
                free(lexer.tokens);
                free(lexer.representations);
                free(lexer.identifiers);
                free(lexer.intern_string_lookup);
                free(lexer.interned_strings);
                return (TokenArray) { NULL, NULL, 0, NULL, NULL, 0 };
            } break;
        }
    }
}
