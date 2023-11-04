#include "lexer.h"
#include "str.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    size_t   count;
    Token*   tokens;
    Str*     representations;
} Lexer;


Str lexer_repr_of(TokenArray tokens, TokenId id) {
    return tokens.representations[id];
}


TokenArray lexer_lex(const char* source) {
    Lexer lexer =  {
        .count  = 0,
        .representations = (Str*) malloc(1024 * sizeof(Str)),
        .tokens = (Token*) malloc(1024),
    };

    while (1) {
        // Skip whitespace.
        while (*source == ' ' || *source == '\t' || *source == '\n' || *source == '\r') {
            ++source;
        }

        switch (*source) {
            case '\0': {
                lexer.tokens[lexer.count++] = (Token) { Token_Eof };
                return (TokenArray) { lexer.tokens, lexer.representations, lexer.count };
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
                    lexer.tokens[lexer.count++] = (Token) { Token_Literal };
                    break;
                }

                fprintf(stderr, "Unknown token: '%c'\n", *source);
                free(lexer.tokens);
                free(lexer.representations);
                return (TokenArray) { NULL, NULL, 0 };
            } break;
        }
    }
}
