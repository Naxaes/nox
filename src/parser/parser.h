#pragma once

#include "types.h"
#include "lexer/lexer.h"


typedef u32 NodeId;
#define NODE_ID_INVALID ((NodeId)0)

typedef enum {
    NodeKind_Invalid,
    NodeKind_Literal,
    NodeKind_Binary,
} NodeKind;

typedef struct {
    NodeKind kind;
    enum {
        Literal_Integer,
        Literal_Real,
    } type;
    union {
        u64 integer;
        f64 real;
    } value;
} NodeLiteral;

typedef struct {
    NodeKind kind;
    NodeId   left;
    NodeId   right;
    char     op;
} NodeBinary;

typedef union {
    NodeKind kind;
    NodeLiteral literal;
    NodeBinary  binary;
} Node;

typedef struct {
    Node* nodes;
    NodeId start;
} UntypedAst;

UntypedAst parse(TokenArray tokens);
