#pragma once

#include "types.h"
#include "lexer/lexer.h"


typedef enum {
    NodeKind_Invalid,
    NodeKind_Literal,
} NodeKind;

typedef struct {
    NodeKind kind;
    union {
        u64 integer;
        f64 real;
    } value;
} NodeLiteral;

typedef union {
    NodeKind kind;
    NodeLiteral literal;
} Node;

typedef struct {
    Node* nodes;
} UntypedAst;

UntypedAst parse(TokenArray tokens);
