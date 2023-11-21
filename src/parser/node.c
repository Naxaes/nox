#include "node.h"


const char* literal_type_name(LiteralType type) {
#define X(upper, lower, repr) case LiteralType_##upper: return #lower;
    switch (type) {
        ALL_LITERAL_TYPES(X)
    }
#undef X
}

const char* literal_type_repr(LiteralType type) {
#define X(upper, lower, repr) case LiteralType_##upper: return #repr;
    switch (type) {
        ALL_LITERAL_TYPES(X)
    }
#undef X
}

const char* binary_op_name(BinaryOp op) {
#define X(upper, lower, repr, group) case BinaryOp_##upper: return #lower;
    switch (op) {
        ALL_BINARY_OPS(X)
    }
#undef X
}

const char* binary_op_repr(BinaryOp op) {
#define X(upper, lower, repr, group) case BinaryOp_##upper: return #repr;
    switch (op) {
        ALL_BINARY_OPS(X)
    }
#undef X
}

BinaryOpGroup binary_op_group(BinaryOp op) {
#define X(upper, lower, repr, group) case BinaryOp_##upper: return BinaryOpGroup_##group;
    switch (op) {
        ALL_BINARY_OPS(X)
    }
#undef X
}


/* ---------------------------- NODE HELPERS -------------------------------- */
int node_is_expression(const Node* node) {
#define X(upper, lower, flags, body) case NodeKind_##upper: return (flags) & NodeFlag_Is_Expression;
    switch (node->kind) {
        ALL_NODES(X)
    }
#undef X
}

int node_is_statement(const Node* node) {
#define X(upper, lower, flags, body) case NodeKind_##upper: return (flags) & NodeFlag_Is_Statement;
    switch (node->kind) {
        ALL_NODES(X)
    }
#undef X
}

int node_is_constant(const Node* node) {
#define X(upper, lower, flags, body) case NodeKind_##upper: return (flags) & NodeFlag_Is_Constant;
    switch (node->kind) {
        ALL_NODES(X)
    }
#undef X
}


/* ---------------------------- BINARY OP HELPERS -------------------------------- */
int binary_op_is_arithmetic(BinaryOp op) {
#define X(upper, lower, repr, group) case BinaryOp_##upper: return BinaryOpGroup_##group == BinaryOpGroup_Arithmetic;
    switch (op) {
        ALL_BINARY_OPS(X)
    }
#undef X
}

int binary_op_is_relational(BinaryOp op) {
#define X(upper, lower, repr, group) case BinaryOp_##upper: return BinaryOpGroup_##group == BinaryOpGroup_Relational;
    switch (op) {
        ALL_BINARY_OPS(X)
    }
#undef X
}
