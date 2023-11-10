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
    NodeKind_Assign,
    NodeKind_VarDecl,
    NodeKind_Block,
    NodeKind_If,
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
        Literal_Boolean,
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
    enum {
        Binary_Operation_Add,
        Binary_Operation_Sub,
        Binary_Operation_Mul,
        Binary_Operation_Div,
        Binary_Operation_Mod,
        Binary_Operation_Lt,
        Binary_Operation_Le,
        Binary_Operation_Eq,
        Binary_Operation_Ne,
        Binary_Operation_Ge,
        Binary_Operation_Gt,
    } op;
} NodeBinary;
const char* binary_op_repr(NodeBinary binary);
int binary_op_is_arithmetic(NodeBinary binary);
int binary_op_is_comparison(NodeBinary binary);

typedef struct {
    NodeBase base;
    const char* name;
    Node* expression;
} NodeAssign;

typedef struct {
    NodeBase base;
    const char* name;
    Node* expression;
} NodeVarDecl;

typedef struct {
    NodeBase base;
    Node**   nodes;
} NodeBlock;

typedef struct {
    NodeBase   base;
    Node*      condition;
    NodeBlock* then_block;
} NodeIf;

union Node {
    NodeKind        kind;
    NodeBase        base;
    NodeLiteral     literal;
    NodeIdentifier  identifier;
    NodeBinary      binary;
    NodeAssign      assign;
    NodeVarDecl     var_decl;
    NodeBlock       block;
    NodeIf          if_stmt;
};

int node_is_expression(Node* node);
// NOTE(ted): All nodes are statements except for an invalid node.
int node_is_statement(Node* node);


typedef struct {
    TokenArray tokens;

    Node*  nodes;
    Node*  start;
    Node** views;
} UntypedAst;

UntypedAst parse(TokenArray tokens);

void grammar_tree_free(UntypedAst ast);
