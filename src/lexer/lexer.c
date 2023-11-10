#include "lexer.h"
#include "str.h"
#include "error.h"
#include "utf8.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>


typedef struct {
    Str source;

    size_t   count;
    Token*   tokens;
    IdentId* identifiers;
    size_t*  indices;

    u32*   intern_string_lookup;
    u8*    interned_strings;
    size_t interned_string_size;
} Lexer;


void lexer_free(Lexer* lexer) {
    free(lexer->tokens);
    free(lexer->identifiers);
    free(lexer->indices);
    free(lexer->intern_string_lookup);
    free(lexer->interned_strings);
}

void token_array_free(TokenArray tokens) {
    free(tokens.tokens);
    free(tokens.identifiers);
    free(tokens.indices);
    free(tokens.interned_strings);
}

TokenArray lexer_to_token_array(Lexer* lexer, Str name) {
    free(lexer->intern_string_lookup);
    return (TokenArray) {
            name,
            lexer->source,
            lexer->tokens,
            lexer->identifiers,
            lexer->indices,
            lexer->count,
            lexer->interned_strings,
            lexer->interned_string_size
    };
}


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
        if (strncmp(string.data, candidate, string.size) == 0) {
            // We found a match! Returned the cached offset.
            return offset;
        } else {
            // Go to the next entry in the lookup table.
            index = (index + 1) & (1024-1);
            offset = lexer->intern_string_lookup[index];
        }
    } while (1);
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

static inline Str parse_number(const char* source, Token* token) {
    const char* start = source;
    do {
        ++source;
    } while (is_digit(*source));
    const char* end = source;

    if (*source == '.') {
        do {
            ++source;
        } while (is_digit(*source));
        end = source;
        *token = Token_Real;
        return (Str) { (size_t)(end-start), start };
    }

    *token = Token_Number;
    return (Str) { (size_t)(end-start), start };
}

static inline int add_single_token(Lexer* lexer, const char* current, Token token) {
    lexer->indices[lexer->count] = current - lexer->source.data;
    lexer->tokens[lexer->count++] = (Token) { token };
    return 1;
}

static inline int add_double_token(Lexer* lexer, const char* current, Token token) {
    lexer->indices[lexer->count] = current - lexer->source.data;
    lexer->tokens[lexer->count++] = (Token) { token };
    return 2;
}

static inline int add_token_with_identifier(Lexer* lexer, const char* current, Token token, Str identifier) {
    IdentId ident = intern_string(lexer, identifier);
    lexer->indices[lexer->count] = current - lexer->source.data;
    lexer->identifiers[lexer->count] = ident;
    lexer->tokens[lexer->count++] = (Token) { token };
    return identifier.size;
}

static Token token_from_string(Str string) {
    if (str_equals(string, STR("if")))
        return Token_If;
    return Token_Identifier;
}

const char* lexer_repr_of(TokenArray tokens, TokenId id) {
    Token token = tokens.tokens[id];
    switch (token) {
        case Token_Invalid:           return "<Invalid>";
        case Token_Plus:              return "+";
        case Token_Asterisk:          return "*";
        case Token_Equal:             return "=";
        case Token_Eof:               return "<EOF>";
        case Token_Minus:             return "-";
        case Token_Slash:             return "/";
        case Token_Percent:           return "%";
        case Token_Less:              return "<";
        case Token_Less_Equal:        return "<=";
        case Token_Equal_Equal:       return "==";
        case Token_Exclamation_Equal: return "!=";
        case Token_Greater_Equal:     return ">=";
        case Token_Greater:           return ">";
        case Token_Exclamation:       return "!";
        case Token_Open_Brace:        return "{";
        case Token_Close_Brace:        return "}";
        case Token_If:                return "if";

        case Token_Number:
        case Token_Real:
        case Token_Identifier: {
            size_t offset = tokens.identifiers[id];
            const char* repr = (const char*) &tokens.interned_strings[offset];
            return repr;
        }
    }
}


TokenArray lexer_lex(Str name, Str source) {
    Lexer lexer =  {
        .source = source,
        .count  = 0,
        .tokens = (Token*) malloc(1024 * sizeof(Token)),
        .identifiers = (IdentId*) malloc(1024 * sizeof(IdentId)),
        .indices = (size_t*) malloc(1024 * sizeof(size_t)),
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
                add_single_token(&lexer, current, Token_Eof);
                return lexer_to_token_array(&lexer, name);
            } break;
            case '+': {
                current += add_single_token(&lexer, current, Token_Plus);
            } break;
            case '*': {
                current += add_single_token(&lexer, current, Token_Asterisk);
            } break;
            case '/': {
                current += add_single_token(&lexer, current, Token_Slash);
            } break;
            case '%': {
                current += add_single_token(&lexer, current, Token_Percent);
            } break;
            case '>': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Greater_Equal);
                else
                    current += add_single_token(&lexer, current, Token_Greater);
            } break;
            case ':': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Colon_Equal);
                else
                    assert(0 && "Not implemented");
            } break;
            case '=': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Equal_Equal);
                else
                    current += add_single_token(&lexer, current, Token_Equal);
            } break;
            case '!': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Exclamation_Equal);
                else
                    current += add_single_token(&lexer, current, Token_Exclamation);
            } break;
            case '<': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Less_Equal);
                else
                    current += add_single_token(&lexer, current, Token_Less);
            } break;
            case '{': {
                current += add_single_token(&lexer, current, Token_Open_Brace);
            } break;
            case '}': {
                current += add_single_token(&lexer, current, Token_Close_Brace);
            } break;
            default: {
                if (is_digit(*current)) {
                    Token token;
                    Str string = parse_number(current, &token);
                    current += add_token_with_identifier(&lexer, current, token, string);
                } else if (is_identifier_start(*current)) {
                    Str string = parse_identifier(current);
                    Token keyword_or_ident = token_from_string(string);
                    current += add_token_with_identifier(&lexer, current, keyword_or_ident, string);
                } else {
                    int width = multi_byte_count(*current);
                    if (width == 0) width = 1;
                    fprintf(stderr, "[Error] (Lexer) " STR_FMT "\n  Unknown character: '" STR_FMT "'\n", STR_ARG(name), width, current);

                    // Assume all characters only take up one column in the terminal for now.
                    int start = (int)(current - source.data);
                    point_to_error(source, start, start+width);
                    lexer_free(&lexer);
                    return (TokenArray) { name, source, NULL, NULL, NULL, 0, NULL, 0 };
                }
            } break;
        }
    }
}

