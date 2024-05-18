#ifndef NODE_H
#define NODE_H

#include "lexer/lexer.h"


typedef enum LiteralType {
    LiteralType_Void,
    LiteralType_Boolean,
    LiteralType_Integer,
    LiteralType_Real,
    LiteralType_String,
} LiteralType;
#define LITERAL_TYPE_COUNT 5

typedef union LiteralValue {
    u64 integer;
    f64 real;
    const char* string;
} LiteralValue;

typedef enum BinaryOpGroup {
    BinaryOpGroup_Arithmetic,
    BinaryOpGroup_Relational,
    BinaryOpGroup_Logical,
    BinaryOpGroup_Bitwise,
} BinaryOpGroup;
#define BINARY_OP_GROUP_COUNT 4

typedef enum BinaryOp {
    BinaryOp_Add,
    BinaryOp_Sub,
    BinaryOp_Mul,
    BinaryOp_Div,
    BinaryOp_Mod,
    BinaryOp_Lt,
    BinaryOp_Le,
    BinaryOp_Eq,
    BinaryOp_Ne,
    BinaryOp_Ge,
    BinaryOp_Gt,
} BinaryOp;
#define BINARY_OP_COUNT 11

typedef enum NodeFlag {
    NodeFlag_None           = 0,
    NodeFlag_Is_Expression  = 1 << 0,
    NodeFlag_Is_Statement   = 1 << 1,
    NodeFlag_Is_Constant    = 1 << 2,
} NodeFlag;

typedef enum NodeKind {
    NodeKind_Literal,
    NodeKind_Identifier,
    NodeKind_Binary,
    NodeKind_Call,
    NodeKind_Type,
    NodeKind_Assign,
    NodeKind_VarDecl,
    NodeKind_Block,
    NodeKind_FunBody,
    NodeKind_FunParam,
    NodeKind_FunDecl,
    NodeKind_Return,
    NodeKind_If,
    NodeKind_While,
    NodeKind_Module,
} NodeKind;
#define NODE_KIND_COUNT 14

typedef u32 NodeId;
typedef union Node Node;

typedef struct NodeBase {
    NodeKind   kind;
    TokenIndex start;
    TokenIndex end;
} NodeBase;

typedef struct NodeLiteral {
    NodeBase        base;
    LiteralType     type;
    LiteralValue    value;
} NodeLiteral;

typedef struct NodeIdentifier {
    NodeBase        base;
    const char*     name;
} NodeIdentifier;

typedef struct NodeBinary {
    NodeBase    base;
    Node*       left;
    Node*       right;
    BinaryOp    op;
} NodeBinary;

typedef struct {
    NodeBase    base;
    const char* name;
    i32         count;
    Node**      args;
} NodeCall;

typedef struct {
    NodeBase    base;
    const char* name;
} NodeType;

typedef struct {
    NodeBase    base;
    const char* name;
    Node*       expression;
} NodeAssign;

typedef struct {
    NodeBase    base;
    const char* name;
    i32         decl_offset;
    Node*       expression;
} NodeVarDecl;

typedef struct {
    NodeBase    base;
    i32         id;
    i32         parent;
    i32         count;
    Node**      nodes;
} NodeBlock;

typedef struct {
    NodeBase    base;
    i32         id;
    i32         parent;
    i32         count;
    Node**      nodes;
    i32         decls;
} NodeFunBody;

typedef struct {
    NodeBase    base;
    const char* name;
    int         offset;
    Node*       type;
    Node*       expression;
} NodeFunParam;

typedef struct {
    NodeBase        base;
    const char*     name;
    NodeFunParam**  params;
    i32             param_count;
    Node*           return_type;
    NodeFunBody*    body;
} NodeFunDecl;

typedef struct {
    NodeBase    base;
    Node*       expression;
} NodeReturn;

typedef struct {
    NodeBase    base;
    Node*       condition;
    NodeBlock*  then_block;
    NodeBlock*  else_block;
} NodeIf;

typedef struct {
    NodeBase    base;
    Node*       condition;
    NodeBlock*  then_block;
    NodeBlock*  else_block;
} NodeWhile;

typedef struct {
    NodeBase    base;
    Node**      stmts;
    i64         stmt_count;
    Node**      decls;
    i64         decl_count;
    i64         global_count;
} NodeModule;


STATIC_ASSERT(NODE_KIND_COUNT == 14, NodeKind_count_has_changed);
union Node {
    NodeKind        kind;
    NodeBase        base;
    union As {
        NodeLiteral literal;
        NodeIdentifier identifier;
        NodeBinary binary;
        NodeCall call;
        NodeType type;
        NodeAssign assign;
        NodeVarDecl var_decl;
        NodeBlock block;
        NodeFunBody fun_body;
        NodeFunParam fun_param;
        NodeFunDecl fun_decl;
        NodeReturn return_stmt;
        NodeIf if_stmt;
        NodeWhile while_stmt;
        NodeModule module;
    } as;
};




#define visit_impl(visitor, node) do { \
    switch (node->kind) {                                                                         \
        case NodeKind_Literal:                                                                    \
            return visitor->visit_literal(visitor, (NodeLiteral*) node);                          \
        case NodeKind_Identifier:                                                                 \
            return visitor->visit_identifier(visitor, (NodeIdentifier*) node);                    \
        case NodeKind_Binary:                                                                     \
            return visitor->visit_binary(visitor, (NodeBinary*) node);                            \
        case NodeKind_Call:                                                                       \
            return visitor->visit_call(visitor, (NodeCall*) node);                                \
        case NodeKind_Type:                                                                       \
            return visitor->visit_type(visitor, (NodeType*) node);                                \
        case NodeKind_Assign:                                                                     \
            return visitor->visit_assign(visitor, (NodeAssign*) node);                            \
        case NodeKind_VarDecl:                                                                    \
            return visitor->visit_var_decl(visitor, (NodeVarDecl*) node);                         \
        case NodeKind_Block:                                                                      \
            return visitor->visit_block(visitor, (NodeBlock*) node);                              \
        case NodeKind_FunParam:                                                                   \
            return visitor->visit_fun_param(visitor, (NodeFunParam*) node);                       \
        case NodeKind_FunBody:                                                                    \
            return visitor->visit_fun_body(visitor, (NodeFunBody*) node);                         \
        case NodeKind_FunDecl:                                                                    \
            return visitor->visit_fun_decl(visitor, (NodeFunDecl*) node);                         \
        case NodeKind_Return:                                                                     \
            return visitor->visit_return_stmt(visitor, (NodeReturn*) node);                       \
        case NodeKind_If:                                                                         \
            return visitor->visit_if_stmt(visitor, (NodeIf*) node);                               \
        case NodeKind_While:                                                                      \
            return visitor->visit_while_stmt(visitor, (NodeWhile*) node);                         \
        case NodeKind_Module:                                                                     \
            return visitor->visit_module(visitor, (NodeModule*) node);                            \
    }                                                                                             \
} while (0)


const char* binary_op_name(BinaryOp op);
const char* binary_op_repr(BinaryOp op);
BinaryOpGroup binary_op_group(BinaryOp op);
int binary_op_is_arithmetic(BinaryOp op);
int binary_op_is_relational(BinaryOp op);

const char* literal_type_name(LiteralType type);
const char* literal_type_repr(LiteralType type);

Str node_kind_name(NodeKind kind);
int node_is_expression(const Node* node);
int node_is_statement(const Node* node);


STATIC_ASSERT(NODE_KIND_COUNT == 14, NodeKind_count_has_changed);
static inline NodeBase node_base_literal     (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Literal,    start,  end }; }
static inline NodeBase node_base_identifier  (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Identifier, start,  end }; }
static inline NodeBase node_base_binary      (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Binary,     start,  end }; }
static inline NodeBase node_base_call        (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Call,       start,  end }; }
static inline NodeBase node_base_type        (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Type,       start,  end }; }
static inline NodeBase node_base_assign      (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Assign,     start,  end }; }
static inline NodeBase node_base_var_decl    (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_VarDecl,    start,  end }; }
static inline NodeBase node_base_block       (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Block,      start,  end }; }
static inline NodeBase node_base_fun_body    (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_FunBody,    start,  end }; }
static inline NodeBase node_base_fun_param   (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_FunParam,   start,  end }; }
static inline NodeBase node_base_fun_decl    (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_FunDecl,    start,  end }; }
static inline NodeBase node_base_return_stmt (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Return,     start,  end }; }
static inline NodeBase node_base_if_stmt     (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_If,         start,  end }; }
static inline NodeBase node_base_while_stmt  (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_While,      start,  end }; }
static inline NodeBase node_base_module      (TokenIndex start, TokenIndex end)  { return (NodeBase) { NodeKind_Module,     start,  end }; }

STATIC_ASSERT(NODE_KIND_COUNT == 14, NodeKind_count_has_changed);
static inline Node node_literal      (NodeLiteral    node) { return (Node) { .as.literal=node    }; }
static inline Node node_identifier   (NodeIdentifier node) { return (Node) { .as.identifier=node }; }
static inline Node node_binary       (NodeBinary     node) { return (Node) { .as.binary=node     }; }
static inline Node node_call         (NodeCall       node) { return (Node) { .as.call=node       }; }
static inline Node node_type         (NodeType       node) { return (Node) { .as.type=node       }; }
static inline Node node_assign       (NodeAssign     node) { return (Node) { .as.assign=node     }; }
static inline Node node_var_decl     (NodeVarDecl    node) { return (Node) { .as.var_decl=node   }; }
static inline Node node_block        (NodeBlock      node) { return (Node) { .as.block=node      }; }
static inline Node node_fun_body     (NodeFunBody    node) { return (Node) { .as.fun_body=node   }; }
static inline Node node_fun_param    (NodeFunParam   node) { return (Node) { .as.fun_param=node  }; }
static inline Node node_fun_decl     (NodeFunDecl    node) { return (Node) { .as.fun_decl=node   }; }
static inline Node node_return_stmt  (NodeReturn     node) { return (Node) { .as.return_stmt=node}; }
static inline Node node_if_stmt      (NodeIf         node) { return (Node) { .as.if_stmt=node    }; }
static inline Node node_while_stmt   (NodeWhile      node) { return (Node) { .as.while_stmt=node }; }
static inline Node node_module       (NodeModule     node) { return (Node) { .as.module=node     }; }

#define node_expect(node, kind_) ((node)->kind == NodeKind_##kind_ ? (Node##kind_*) (node) : (panic(0, "Expected node of kind %s, got %s", #kind_, node_kind_name((node)->kind)), NULL))

#endif  // NODE_H


#ifdef NODE_IMPLEMENTATION
const char* literal_type_name(LiteralType type) {
    STATIC_ASSERT(LITERAL_TYPE_COUNT == 5, LiteralType_count_has_changed);
    switch (type) {
        case LiteralType_Void:
            return "void";
        case LiteralType_Boolean:
            return "boolean";
        case LiteralType_Integer:
            return "integer";
        case LiteralType_Real:
            return "real";
        case LiteralType_String:
            return "string";
    }
}

const char* literal_type_repr(LiteralType type) {
    STATIC_ASSERT(LITERAL_TYPE_COUNT == 5, LiteralType_count_has_changed);
    switch (type) {
        case LiteralType_Void:
            return "void";
        case LiteralType_Boolean:
            return "bool";
        case LiteralType_Integer:
            return "int";
        case LiteralType_Real:
            return "real";
        case LiteralType_String:
            return "str";
    }
}

const char* binary_op_name(BinaryOp op) {
    STATIC_ASSERT(BINARY_OP_COUNT == 11, BinaryOp_count_has_changed);
    switch (op) {
        case BinaryOp_Add:
            return "add";
        case BinaryOp_Sub:
            return "sub";
        case BinaryOp_Mul:
            return "mul";
        case BinaryOp_Div:
            return "div";
        case BinaryOp_Mod:
            return "mod";
        case BinaryOp_Lt:
            return "lt";
        case BinaryOp_Le:
            return "le";
        case BinaryOp_Eq:
            return "eq";
        case BinaryOp_Ne:
            return "ne";
        case BinaryOp_Ge:
            return "ge";
        case BinaryOp_Gt:
            return "gt";
    }
}

const char* binary_op_repr(BinaryOp op) {
    STATIC_ASSERT(BINARY_OP_COUNT == 11, BinaryOp_count_has_changed);
    switch (op) {
        case BinaryOp_Add:
            return "+";
        case BinaryOp_Sub:
            return "-";
        case BinaryOp_Mul:
            return "*";
        case BinaryOp_Div:
            return "/";
        case BinaryOp_Mod:
            return "%";
        case BinaryOp_Lt:
            return "<";
        case BinaryOp_Le:
            return "<=";
        case BinaryOp_Eq:
            return "==";
        case BinaryOp_Ne:
            return "!=";
        case BinaryOp_Ge:
            return ">=";
        case BinaryOp_Gt:
            return ">";
    }
}

BinaryOpGroup binary_op_group(BinaryOp op) {
    STATIC_ASSERT(BINARY_OP_COUNT == 11, BinaryOp_count_has_changed);
    switch (op) {
        case BinaryOp_Add:
            return BinaryOpGroup_Arithmetic;
        case BinaryOp_Sub:
            return BinaryOpGroup_Arithmetic;
        case BinaryOp_Mul:
            return BinaryOpGroup_Arithmetic;
        case BinaryOp_Div:
            return BinaryOpGroup_Arithmetic;
        case BinaryOp_Mod:
            return BinaryOpGroup_Arithmetic;
        case BinaryOp_Lt:
            return BinaryOpGroup_Relational;
        case BinaryOp_Le:
            return BinaryOpGroup_Relational;
        case BinaryOp_Eq:
            return BinaryOpGroup_Relational;
        case BinaryOp_Ne:
            return BinaryOpGroup_Relational;
        case BinaryOp_Ge:
            return BinaryOpGroup_Relational;
        case BinaryOp_Gt:
            return BinaryOpGroup_Relational;
    }
}


/* ---------------------------- NODE HELPERS -------------------------------- */
int node_is_expression(const Node* node) {
#define X(upper, lower, flags, body) case NodeKind_##upper: return (flags) & NodeFlag_Is_Expression;
    switch (node->kind) {
        case NodeKind_Literal:
            return (NodeFlag_Is_Expression | NodeFlag_Is_Constant) & NodeFlag_Is_Expression;
        case NodeKind_Identifier:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Expression;
        case NodeKind_Binary:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Expression;
        case NodeKind_Call:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Expression;
        case NodeKind_Type:
            return (NodeFlag_None) & NodeFlag_Is_Expression;
        case NodeKind_Assign:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_VarDecl:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_Block:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_FunBody:
            return (NodeFlag_None) & NodeFlag_Is_Expression;
        case NodeKind_FunParam:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_FunDecl:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_Return:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_If:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_While:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Expression;
        case NodeKind_Module:
            return (NodeFlag_None) & NodeFlag_Is_Expression;
    }
#undef X
}

int node_is_statement(const Node* node) {
#define X(upper, lower, flags, body) case NodeKind_##upper: return (flags) & NodeFlag_Is_Statement;
    switch (node->kind) {
        case NodeKind_Literal:
            return (NodeFlag_Is_Expression | NodeFlag_Is_Constant) & NodeFlag_Is_Statement;
        case NodeKind_Identifier:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Statement;
        case NodeKind_Binary:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Statement;
        case NodeKind_Call:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Statement;
        case NodeKind_Type:
            return (NodeFlag_None) & NodeFlag_Is_Statement;
        case NodeKind_Assign:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_VarDecl:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_Block:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_FunBody:
            return (NodeFlag_None) & NodeFlag_Is_Statement;
        case NodeKind_FunParam:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_FunDecl:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_Return:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_If:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_While:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Statement;
        case NodeKind_Module:
            return (NodeFlag_None) & NodeFlag_Is_Statement;
    }
#undef X
}

int node_is_constant(const Node* node) {
#define X(upper, lower, flags, body) case NodeKind_##upper: return (flags) & NodeFlag_Is_Constant;
    switch (node->kind) {
        case NodeKind_Literal:
            return (NodeFlag_Is_Expression | NodeFlag_Is_Constant) & NodeFlag_Is_Constant;
        case NodeKind_Identifier:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Constant;
        case NodeKind_Binary:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Constant;
        case NodeKind_Call:
            return (NodeFlag_Is_Expression) & NodeFlag_Is_Constant;
        case NodeKind_Type:
            return (NodeFlag_None) & NodeFlag_Is_Constant;
        case NodeKind_Assign:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_VarDecl:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_Block:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_FunBody:
            return (NodeFlag_None) & NodeFlag_Is_Constant;
        case NodeKind_FunParam:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_FunDecl:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_Return:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_If:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_While:
            return (NodeFlag_Is_Statement) & NodeFlag_Is_Constant;
        case NodeKind_Module:
            return (NodeFlag_None) & NodeFlag_Is_Constant;
    }
#undef X
}


/* ---------------------------- BINARY OP HELPERS -------------------------------- */
int binary_op_is_arithmetic(BinaryOp op) {
    STATIC_ASSERT(BINARY_OP_COUNT == 11, BinaryOp_count_has_changed);
    switch (op) {
        case BinaryOp_Add:
        case BinaryOp_Sub:
        case BinaryOp_Mul:
        case BinaryOp_Div:
        case BinaryOp_Mod:
            return true;
        case BinaryOp_Lt:
        case BinaryOp_Le:
        case BinaryOp_Eq:
        case BinaryOp_Ne:
        case BinaryOp_Ge:
        case BinaryOp_Gt:
            return false;
    }
}

int binary_op_is_relational(BinaryOp op) {
    STATIC_ASSERT(BINARY_OP_COUNT == 11, BinaryOp_count_has_changed);
    switch (op) {
        case BinaryOp_Add:
        case BinaryOp_Sub:
        case BinaryOp_Mul:
        case BinaryOp_Div:
        case BinaryOp_Mod:
            return false;
        case BinaryOp_Lt:
        case BinaryOp_Le:
        case BinaryOp_Eq:
        case BinaryOp_Ne:
        case BinaryOp_Ge:
        case BinaryOp_Gt:
            return true;
    }
}


Str node_kind_name(NodeKind kind) {
    STATIC_ASSERT(NODE_KIND_COUNT == 14, NodeKind_count_has_changed);
    switch (kind) {
        case NodeKind_Literal:
            return STR("Literal");
        case NodeKind_Identifier:
            return STR("Identifier");
        case NodeKind_Binary:
            return STR("Binary");
        case NodeKind_Call:
            return STR("Call");
        case NodeKind_Type:
            return STR("Type");
        case NodeKind_Assign:
            return STR("Assign");
        case NodeKind_VarDecl:
            return STR("VarDecl");
        case NodeKind_Block:
            return STR("Block");
        case NodeKind_FunBody:
            return STR("FunBody");
        case NodeKind_FunParam:
            return STR("FunParam");
        case NodeKind_FunDecl:
            return STR("FunDecl");
        case NodeKind_Return:
            return STR("Return");
        case NodeKind_If:
            return STR("If");
        case NodeKind_While:
            return STR("While");
        case NodeKind_Module:
            return STR("Module");
    }
}

#endif  // NODE_IMPLEMENTATION
