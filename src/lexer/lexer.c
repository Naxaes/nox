#include "lexer.h"
#include "str.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
    size_t   count;
    Token*   tokens;
    IdentId* identifiers;

    u32*   intern_string_lookup;
    u8*    interned_strings;
    size_t interned_string_size;
} Lexer;

static IdentId intern_string(Lexer* lexer, Str string) {
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
            lexer->intern_string_lookup[index] = offset;
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

static inline int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static inline int is_digit(char c) {
    return '0' <= c && c <= '9';
}

static inline int is_identifier_start(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static inline int is_identifier_continue(char c) {
    return is_identifier_start(c) || is_digit(c);
}

static inline Str parse_identifier(const char* source) {
    const char* start = source;
    do {
        ++source;
    } while (is_identifier_continue(*source));
    const char* end = source;

    return (Str) { (size_t)(end-start), start };
}

static inline Str parse_number(const char* source) {
    const char* start = source;
    do {
        ++source;
    } while (is_digit(*source));
    const char* end = source;

    return (Str) { (size_t)(end-start), start };
}

static inline int add_token(Lexer* lexer, Token token) {
    lexer->tokens[lexer->count++] = (Token) { token };
    return 1;
}

static inline int add_token_with_identifier(Lexer* lexer, Token token, Str identifier) {
    IdentId ident = intern_string(lexer, identifier);
    lexer->identifiers[lexer->count] = ident;
    lexer->tokens[lexer->count++] = (Token) { token };
    return identifier.size;
}

const char* lexer_repr_of(TokenArray tokens, TokenId id) {
    Token token = tokens.tokens[id];
    switch (token) {
        case Token_Invalid:  return "<Invalid>";
        case Token_Plus:     return "+";
        case Token_Asterisk: return "*";
        case Token_Equal:    return "=";
        case Token_Eof:      return "<EOF>";

        case Token_Number:
        case Token_Identifier: {
            size_t offset = tokens.identifiers[id];
            const char* repr = (const char*) &tokens.interned_strings[offset];
            return repr;
        }
    }
}


TokenArray lexer_lex(Str source) {
    Lexer lexer =  {
        .count  = 0,
        .tokens = (Token*) malloc(1024),
        .identifiers = (IdentId*) malloc(1024 * sizeof(IdentId)),
        .intern_string_lookup = memset(malloc(1024 * sizeof(u32)), 0, 1024 * sizeof(u32)),
        .interned_strings     = (u8*) malloc(1024),
        .interned_string_size = sizeof(Str),  // Skip the first few bytes so that 0 from the lookup table means "not found".
    };

    const char* current = source.data;
    while (1) {
        // Skip whitespace.
        while (is_whitespace(*current)) {
            ++current;
        }

        switch (*current) {
            case '\0': {
                add_token(&lexer, Token_Eof);
                free(lexer.intern_string_lookup);
                return (TokenArray) {
                    lexer.tokens,
                    lexer.identifiers,
                    lexer.count,
                    lexer.interned_strings,
                    lexer.interned_string_size
                };
            } break;
            case '+': {
                current += add_token(&lexer, Token_Plus);
            } break;
            case '*': {
                current += add_token(&lexer, Token_Asterisk);
            } break;
            case '=': {
                current += add_token(&lexer, Token_Equal);
            } break;
            default: {
                if (is_digit(*current)) {
                    Str string = parse_number(current);
                    current += add_token_with_identifier(&lexer, Token_Number, string);
                } else if (is_identifier_start(*current)) {
                    Str string = parse_identifier(current);
                    current += add_token_with_identifier(&lexer, Token_Identifier, string);
                } else {
                    fprintf(stderr, "Unknown token: '%c'\n", *current);
                    free(lexer.tokens);
                    free(lexer.identifiers);
                    free(lexer.intern_string_lookup);
                    free(lexer.interned_strings);
                    return (TokenArray) { NULL, NULL, 0, NULL, 0 };
                }
            } break;
        }
    }
}
