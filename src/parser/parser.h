#pragma once

#include "types.h"
#include "lexer/lexer.h"


typedef u32 NodeId;
#define NODE_ID_INVALID ((NodeId)0)

typedef union Node Node;

typedef enum {
    NodeKind_Invalid,
    NodeKind_Literal,
    NodeKind_Identifier,
    NodeKind_Binary,
    NodeKind_VarDecl,
    NodeKind_Block,
} NodeKind;

typedef struct {
    NodeKind kind;
    TokenId  start;
    TokenId  end;
} NodeBase;

typedef struct {
    NodeBase base;
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
    NodeBase base;
    const char* name;
} NodeIdentifier;

typedef struct {
    NodeBase base;
    Node*    left;
    Node*    right;
    char     op;
} NodeBinary;

typedef struct {
    NodeBase base;
    const char* name;
    Node* expression;
} NodeVarDecl;

typedef struct {
    NodeBase base;
    Node**   nodes;
} NodeBlock;

union Node {
    NodeKind        kind;
    NodeBase        base;
    NodeLiteral     literal;
    NodeIdentifier  identifier;
    NodeBinary      binary;
    NodeVarDecl     var_decl;
    NodeBlock       block;
};

typedef struct {
    TokenArray tokens;

    Node*  nodes;
    Node*  start;
    Node** views;
} UntypedAst;

UntypedAst parse(TokenArray tokens);

void grammar_tree_free(UntypedAst ast);
