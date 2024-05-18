#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lexer.h"
#include "str.h"
#include "error.h"
#include "utf8.h"
#include "allocator.h"



/* ---------------------------- TOKEN ARRAY -------------------------------- */
void token_array_free(TokenArray tokens) {
    free(tokens.tokens);
    free(tokens.identifiers);
    free(tokens.source_offsets);
    free(tokens.data_pool);
}


/* ---------------------------- LEXER HELPERS -------------------------------- */
typedef struct {
    u32*   lookup;
    u8*    data;
    size_t used;
} InternPool;

static InternPool intern_pool_make(void) {
    return (InternPool) {
        .lookup = memset(alloc(0, 1024 * sizeof(u32)), 0, 1024 * sizeof(u32)),
        .data   = (u8*) alloc(0, 1024),
        .used   = sizeof(DataPoolIndex),  // Skip the first few bytes so that 0 from the lookup table means "not found".
    };
}

static DataPoolIndex intern_string(InternPool* intern_pool, Str string) {
    u64 index  = str_hash(string) & (1024-1);
    u32 offset = intern_pool->lookup[index];

    do {
        // If the offset is 0, we have not found a match,
        // which means that the string is not interned yet.
        if (offset == 0) {
            offset = intern_pool->used;

            // Copy over the data and a null pointer at the end.
            memcpy(intern_pool->data + intern_pool->used, string.data, string.size);
            memset(intern_pool->data + intern_pool->used + string.size, '\0', 1);

            intern_pool->used += string.size + 1;
            intern_pool->lookup[index] = offset;
            return offset;
        }

        const char* candidate = (const char*) (intern_pool->data + offset);
        if (strncmp(string.data, candidate, string.size) == 0) {
            // We found a match! Returned the cached offset.
            return offset;
        } else {
            // Go to the next entry in the lookup table.
            index = (index + 1) & (1024-1);
            offset = intern_pool->lookup[index];
        }
    } while (1);
}

static inline int is_identifier_start(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static inline int is_identifier_continue(char c) {
    return is_identifier_start(c) || is_digit(c);
}


/* ---------------------------- LEXER IMPL -------------------------------- */
typedef struct {
    Str source;

    size_t          count;
    Token*          tokens;
    DataPoolIndex*  identifiers;
    SourceIndex*    indices;

    InternPool intern_pool;
} Lexer;

static void lexer_free(Lexer* lexer) {
    free(lexer->tokens);
    free(lexer->identifiers);
    free(lexer->indices);
    free(lexer->intern_pool.lookup);
    free(lexer->intern_pool.data);
}

static TokenArray lexer_to_token_array(Lexer* lexer, Str name) {
    free(lexer->intern_pool.lookup);
    return (TokenArray) {
            .name        = name,
            .source      = lexer->source,
            .tokens      = lexer->tokens,
            .identifiers = lexer->identifiers,
            .source_offsets = lexer->indices,
            .size        = lexer->count,
            .data_pool   = lexer->intern_pool.data,
            .data_pool_size = lexer->intern_pool.used
    };
}

static Str parse_identifier(const char* source) {
    assert(is_identifier_start(*source) && "Expected a start of identifier");
    const char* start = source;
    do {
        ++source;
    } while (is_identifier_continue(*source));
    const char* end = source;

    return (Str) { (size_t)(end-start), start };
}

static Str parse_number(const char* source, Token* token) {
    assert(is_digit(*source) && "Expected a digit");
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

static Str parse_string(const char* source, int* is_valid) {
    assert(*source == '"' && "Expected a quote");
    // Skip the first quote.
    const char* start = source + 1;
    continue_parsing:;
    do {
        ++source;
    } while (*source != '"' && *source != '\0');
    const char* end = source;

    if (*source == '"') {
        if (*(source-1) == '\\')  // Quote is escaped.
            goto continue_parsing;

        ++source;
        *is_valid = 1;
        return (Str) { (size_t)(end-start), start };
    } else {
        fprintf(stderr, "[Error] (Lexer) Unterminated string literal.\n");
        size_t length = (size_t)(end-start);
        point_to_error((Str) { length, start }, 0, length);
        *is_valid = 0;
        return STR_EMPTY;
    }
}

static const char* parse_line_comment(const char* source) {
    assert(*source == '/' && *(source+1) == '/' && "Expected a line comment");
    while (*source != '\n' && *source != '\0')
        ++source;
    return source;
}

static const char* parse_block_comment(const char* source) {
    assert(*source == '/' && *(source+1) == '*' && "Expected a block comment");
    source += 2;

    while (*source != '\0' && !(*source == '*' && *(source+1) == '/')) {
        if (*source == '/' && *(source+1) == '*') {
            source = parse_block_comment(source);
        } else {
            ++source;
        }
    }
    if (*source == '\0') {
        fprintf(stderr, "[Error] (Lexer) Unterminated block comment.\n");
        point_to_error((Str) { 2, source-2 }, 0, 2);
    } else {
        source += 2;
    }
    return source;
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
    DataPoolIndex ident = intern_string(&lexer->intern_pool, identifier);
    lexer->indices[lexer->count] = current - lexer->source.data;
    lexer->identifiers[lexer->count] = ident;
    lexer->tokens[lexer->count++] = (Token) { token };
    return (int) identifier.size;
}

static Token token_from_string(Str string) {
    if (str_equals(string, STR("if")))     return Token_If;
    if (str_equals(string, STR("else")))   return Token_Else;
    if (str_equals(string, STR("fun")))    return Token_Fun;
    if (str_equals(string, STR("while")))  return Token_While;
    if (str_equals(string, STR("return"))) return Token_Return;

    return Token_Identifier;
}

const char* lexer_repr_of(TokenArray tokens, TokenIndex id) {
    Token token = tokens.tokens[id];
    switch (token) {
        case Token_Number:
        case Token_Real:
        case Token_String:
        case Token_Identifier: {
            size_t offset = tokens.identifiers[id];
            return (const char*) &tokens.data_pool[offset];
        }
        default: {
            return token_repr(token);
        }
    }
}


TokenArray lexer_lex(Str name, Str source) {
    Lexer lexer =  {
        .source = source,
        .count  = 0,
        .tokens = (Token*) alloc(0, 1024 * sizeof(Token)),
        .identifiers = (DataPoolIndex*) alloc(0, 1024 * sizeof(DataPoolIndex)),
        .indices = (SourceIndex*) alloc(0, 1024 * sizeof(SourceIndex)),
        .intern_pool = intern_pool_make()
    };

    const char* current = source.data;
    while (1) {
        switch (*current) {
            case '\0': {
                add_single_token(&lexer, current, Token_Eof);
                return lexer_to_token_array(&lexer, name);
            } break;
            case '\n':
            case '\t':
            case '\r':
            case ' ':
                current += 1;  // Skip whitespace.
            break;
            case '+': {
                current += add_single_token(&lexer, current, Token_Plus);
            } break;
            case '-': {
                current += add_single_token(&lexer, current, Token_Minus);
            } break;
            case '*': {
                current += add_single_token(&lexer, current, Token_Asterisk);
            } break;
            case '/': {
                if (*(current+1) == '/') {
                    current = parse_line_comment(current);
                } else if (*(current+1) == '*') {
                    current = parse_block_comment(current);
                } else {
                    current += add_single_token(&lexer, current, Token_Slash);
                }
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
                    current += add_single_token(&lexer, current, Token_Colon);
            } break;
            case '=': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Equal_Equal);
                else
                    current += add_single_token(&lexer, current, Token_Equal);
            } break;
            case '!': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Bang_Equal);
                else
                    current += add_single_token(&lexer, current, Token_Bang);
            } break;
            case '<': {
                if (*(current+1) == '=')
                    current += add_double_token(&lexer, current, Token_Less_Equal);
                else
                    current += add_single_token(&lexer, current, Token_Less);
            } break;
            case '(': {
                current += add_single_token(&lexer, current, Token_Open_Paren);
            } break;
            case ')': {
                current += add_single_token(&lexer, current, Token_Close_Paren);
            } break;
            case '{': {
                current += add_single_token(&lexer, current, Token_Open_Brace);
            } break;
            case '}': {
                current += add_single_token(&lexer, current, Token_Close_Brace);
            } break;
            case ',': {
                current += add_single_token(&lexer, current, Token_Comma);
            } break;
            case '"': {
                int is_valid = 0;
                Str string = parse_string(current, &is_valid);
                if (!is_valid) {
                    lexer_free(&lexer);
                    return (TokenArray) { name, source, NULL, NULL, NULL, 0, NULL, 0 };
                }
                // current + 1 to skip the first quote.
                current += add_token_with_identifier(&lexer, current+1, Token_String, string) + 2;
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

