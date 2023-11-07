#pragma once

#include "types.h"
#include "lexer/lexer.h"


typedef u32 NodeId;
#define NODE_ID_INVALID ((NodeId)0)

typedef union Node Node;

typedef enum {
    NodeKind_Invalid,
    NodeKind_Literal,
    NodeKind_Binary,
    NodeKind_VarDecl,
} NodeKind;

typedef struct {
    NodeKind kind;
    enum {
        Literal_Invalid,
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
    Node*    left;
    Node*    right;
    char     op;
} NodeBinary;

typedef struct {
    NodeKind kind;
    const char* name;
    Node* expression;
} NodeVarDecl;

union Node {
    NodeKind kind;
    NodeLiteral literal;
    NodeBinary  binary;
    NodeVarDecl var_decl;
};

typedef struct {
    Node* nodes;
    Node* start;
} UntypedAst;

UntypedAst parse(TokenArray tokens);
