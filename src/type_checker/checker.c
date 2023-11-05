#include "checker.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


TypeId type_check_expression(UntypedAst ast, Node* node);


TypeId type_check_literal(UntypedAst ast, NodeLiteral literal) {
    (void)ast;
    return literal.type;
}

TypeId type_check_binary(UntypedAst ast, NodeBinary literal) {
    TypeId left = type_check_expression(ast, literal.left);
    TypeId right = type_check_expression(ast, literal.right);
    if (left != right) {
        return 0;
    }
    return 1;
}

TypeId type_check_expression(UntypedAst ast, Node* node) {
    switch (node->kind) {
        case NodeKind_Invalid:
            assert(0 && "Invalid node kind");
        case NodeKind_Literal:
            return type_check_literal(ast, node->literal);
        case NodeKind_Binary:
            return type_check_binary(ast, node->binary);
    }
}


TypedAst type_check(UntypedAst ast) {
    Node* first = ast.start;
    TypeId* types = malloc(sizeof(TypeId) * 1024);

    switch (first->kind) {
        case NodeKind_Invalid:
        case NodeKind_Literal: {
            fprintf(stderr, "Unknown node kind: '%d'\n", first->kind);
            free(types);
            return (TypedAst) { NULL, NULL, 0 };
        }
        case NodeKind_Binary: {
            if (!type_check_binary(ast, first->binary)) {
                fprintf(stderr, "Type error!\n");
                return (TypedAst) { NULL, NULL, 0 };
            }
            return (TypedAst) { ast.nodes, types, first };
        }
    }
}

