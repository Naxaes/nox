#include "checker.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


typedef struct {
    UntypedAst ast;

    Block* blocks;
    size_t block_count;

    Block* current;
} Checker;

void push_block(Checker* checker) {
    size_t parent = -1;
    if (checker->current != NULL) {
        parent = checker->current - checker->blocks;
    }
    checker->current = checker->blocks + checker->block_count++;
    checker->current->parent = parent;
    checker->current->locals = (Local *) malloc(1024 * sizeof(Local));
    checker->current->count = 0;
}


TypeId type_check_expression(Checker* checker, Node* node);


TypeId type_check_literal(Checker* checker, NodeLiteral literal) {
    (void)checker;
    return literal.type;
}

TypeId type_check_identifier(Checker* checker, NodeIdentifier identifier) {
    Block* current = checker->current;
    for (size_t i = 0; i < current->count; ++i) {
        Local* local = current->locals + i;
        assert(local->decl.kind == NodeKind_VarDecl && "Invalid node kind");
        if (local->decl.var_decl.name == identifier.name) {
            return local->type;
        }
    }
    fprintf(stderr, "Unknown identifier: '%s'\n", identifier.name);
    return 0;
}

TypeId type_check_binary(Checker* checker, NodeBinary literal) {
    TypeId left  = type_check_expression(checker, literal.left);
    TypeId right = type_check_expression(checker, literal.right);
    if (left != right) {
        return 0;
    }
    return left;
}

TypeId type_check_var_decl(Checker* checker, NodeVarDecl var_decl) {
    TypeId expr = type_check_expression(checker, var_decl.expression);
    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
        .type = expr,
        .decl = (Node) { .var_decl = var_decl },
    };
    return -1;
}


TypeId type_check_expression(Checker* checker, Node* node) {
    switch (node->kind) {
        case NodeKind_Invalid:
        case NodeKind_VarDecl:
        case NodeKind_Block:
            assert(0 && "Invalid node kind");
            return 0;
        case NodeKind_Literal:
            return type_check_literal(checker, node->literal);
        case NodeKind_Identifier:
            return type_check_identifier(checker, node->identifier);
        case NodeKind_Binary:
            return type_check_binary(checker, node->binary);
    }
}

TypeId type_check_statement(Checker* checker, Node* node) {
    switch (node->kind) {
        case NodeKind_Invalid:
            assert(0 && "Invalid node kind");
        case NodeKind_Literal:
            return type_check_literal(checker, node->literal);
        case NodeKind_Identifier:
            return type_check_identifier(checker, node->identifier);
        case NodeKind_Binary:
            return type_check_binary(checker, node->binary);
        case NodeKind_VarDecl:
            return type_check_var_decl(checker, node->var_decl);
        case NodeKind_Block: {
            NodeBlock block = node->block;
            while ((node = *block.nodes++) != NULL) {
                if (type_check_statement(checker, node) == 0)
                    return 0;
            }
            return -1;
        }
    }
}


TypedAst type_check(UntypedAst ast) {
    Node* node = ast.start;

    Checker checker = {
        .ast = ast,
        .blocks = (Block*) malloc(1024 * sizeof(Block)),
        .block_count = 0,
        .current = NULL,
    };

    push_block(&checker);

    TypeId type = type_check_statement(&checker, node);
    if (type == 0) {
        free(checker.blocks);
        return (TypedAst) { NULL, NULL, NULL };
    }

    return (TypedAst) { ast.nodes, checker.blocks, node };
}

