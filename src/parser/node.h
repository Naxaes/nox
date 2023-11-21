#pragma once
#include "lexer/lexer.h"


#define ALL_LITERAL_TYPES(X) \
    X(Void,    void,     void)   \
    X(Boolean, boolean,  bool)   \
    X(Integer, integer,  int)    \
    X(Real,    real,     real)   \
    X(String,  string,   str)    \


#define X(upper, lower, repr) LiteralType_##upper,
typedef enum {
    ALL_LITERAL_TYPES(X)
} LiteralType;
#undef X
const char* literal_type_name(LiteralType type);
const char* literal_type_repr(LiteralType type);


typedef union {
    u64 integer;
    f64 real;
    const char* string;
} LiteralValue;

typedef enum {
    BinaryOpGroup_Arithmetic,
    BinaryOpGroup_Relational,
    BinaryOpGroup_Logical,
    BinaryOpGroup_Bitwise,
} BinaryOpGroup;

#define ALL_BINARY_OPS(X) \
    X(Add, add, +,   Arithmetic)  \
    X(Sub, sub, -,   Arithmetic)  \
    X(Mul, mul, *,   Arithmetic)  \
    X(Div, div, /,   Arithmetic)  \
    X(Mod, mod, %,   Arithmetic)  \
    X(Lt,  lt,  <,   Relational)  \
    X(Le,  le,  <=,  Relational)  \
    X(Eq,  eq,  ==,  Relational)  \
    X(Ne,  ne,  !=,  Relational)  \
    X(Ge,  ge,  >=,  Relational)  \
    X(Gt,  gt,  >,   Relational)  \

#define X(upper, lower, repr, group) BinaryOp_##upper,
typedef enum {
    ALL_BINARY_OPS(X)
} BinaryOp;
#undef X
const char* binary_op_name(BinaryOp op);
const char* binary_op_repr(BinaryOp op);
BinaryOpGroup binary_op_group(BinaryOp op);
int binary_op_is_arithmetic(BinaryOp op);
int binary_op_is_relational(BinaryOp op);


typedef enum {
    NodeFlag_None           = 0,
    NodeFlag_Is_Expression  = 1 << 0,
    NodeFlag_Is_Statement   = 1 << 1,
    NodeFlag_Is_Constant    = 1 << 2,
} NodeFlag;


#define ALL_NODES(X) \
    X(Literal, literal, NodeFlag_Is_Expression|NodeFlag_Is_Constant,    \
        LiteralValue value;                                             \
        LiteralType  type;                                              \
    )                                                                   \
    X(Identifier, identifier, NodeFlag_Is_Expression,                   \
        const char* name;                                               \
    )                                                                   \
    X(Binary, binary, NodeFlag_Is_Expression,                           \
        Node* left;                                                     \
        Node* right;                                                    \
        BinaryOp op;                                                    \
    )                                                                   \
    X(Call, call, NodeFlag_Is_Expression,                               \
        const char* name;                                               \
        i32    count;                                                   \
        Node** args;                                                    \
    )                                                                   \
    X(Type, type, NodeFlag_None,                                        \
        const char* name;                                               \
    )                                                                   \
    X(Assign, assign, NodeFlag_Is_Statement,                            \
        const char* name;                                               \
        Node* expression;                                               \
    )                                                                   \
    X(VarDecl, var_decl, NodeFlag_Is_Statement,                         \
        const char* name;                                               \
        i32 decl_offset;                                                \
        Node* expression;                                               \
    )                                                                   \
    X(Block, block, NodeFlag_Is_Statement,                              \
        i32    id;                                                      \
        i32    parent;                                                  \
        i32    count;                                                   \
        Node** nodes;                                                   \
    )                                                                   \
    X(FunBody, fun_body, NodeFlag_None,                                 \
        i32    id;                                                      \
        i32    parent;                                                  \
        i32    count;                                                   \
        Node** nodes;                                                   \
        i32    decls;                                                   \
    )                                                                   \
    X(FunParam, fun_param, NodeFlag_Is_Statement,                       \
        const char* name;                                               \
        int   offset;                                                   \
        Node* type;                                                     \
        Node* expression;                                               \
    )                                                                   \
    X(FunDecl, fun_decl, NodeFlag_Is_Statement,                         \
        const char* name;                                               \
        NodeFunParam** params;                                          \
        i32 param_count;                                                \
        Node* return_type;                                              \
        NodeFunBody* body;                                              \
    )                                                                   \
    X(Return, return_stmt, NodeFlag_Is_Statement,                       \
        Node* expression;                                               \
    )                                                                   \
    X(If, if_stmt, NodeFlag_Is_Statement,                               \
        Node* condition;                                                \
        NodeBlock* then_block;                                          \
        NodeBlock* else_block;                                          \
    )                                                                   \
    X(While, while_stmt, NodeFlag_Is_Statement,                         \
        Node* condition;                                                \
        NodeBlock* then_block;                                          \
        NodeBlock* else_block;                                          \
    )                                                                   \
    X(Module, module, NodeFlag_None,                                    \
        Node** stmts;                                                   \
        i64    stmt_count;                                              \
        Node** decls;                                                   \
        i64    decl_count;                                              \
        i64    global_count;                                            \
    )                                                                   \


typedef enum {
#define X(upper, lower, flags, body) NodeKind_##upper,
    ALL_NODES(X)
#undef X
} NodeKind;

typedef u32 NodeId;

typedef union Node Node;
typedef struct {
    NodeKind   kind;
    TokenIndex start;
    TokenIndex end;
} NodeBase;

#define X(upper, lower, flags, body) typedef struct { NodeBase base; body } Node##upper;
ALL_NODES(X)
#undef X

union Node {
    NodeKind kind;
    NodeBase base;
#define X(upper, lower, flags, body) Node##upper lower;
    ALL_NODES(X)
#undef X
};

#define X(upper, lower, flags, body) \
    static inline NodeBase node_base_##lower(TokenIndex start, TokenIndex end) { return (NodeBase) { NodeKind_##upper, start, end }; }
ALL_NODES(X)
#undef X

#define X(upper, lower, flags, body) \
    static inline Node node_##lower(Node##upper node) { return (Node) { .lower = node }; }
ALL_NODES(X)
#undef X

int node_is_expression(const Node* node);
int node_is_statement(const Node* node);
