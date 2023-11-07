#include "checker.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


typedef struct {
    UntypedAst ast;

    TypeId* types;
    size_t  type_count;

    Node*   decls;
    size_t  decl_count;
} Checker;


TypeId type_check_expression(Checker* checker, Node* node);


TypeId type_check_literal(Checker* checker, NodeLiteral literal) {
    (void)checker;
    return literal.type;
}

TypeId type_check_binary(Checker* checker, NodeBinary literal) {
    TypeId left = type_check_expression(checker, literal.left);
    TypeId right = type_check_expression(checker, literal.right);
    if (left != right) {
        return -1;
    }
    return left;
}

TypeId type_check_var_decl(Checker* checker, NodeVarDecl var_decl) {
    TypeId expr = type_check_expression(checker, var_decl.expression);
    checker->types[checker->type_count++] = expr;
    checker->decls[checker->decl_count++] = (Node) { .var_decl = var_decl };
    return 0;
}


TypeId type_check_expression(Checker* checker, Node* node) {
    switch (node->kind) {
        case NodeKind_Invalid:
            assert(0 && "Invalid node kind");
        case NodeKind_Literal:
            return type_check_literal(checker, node->literal);
        case NodeKind_Binary:
            return type_check_binary(checker, node->binary);
        case NodeKind_VarDecl:
            return type_check_var_decl(checker, node->var_decl);
    }
}


TypedAst type_check(UntypedAst ast) {
    Node* first = ast.start;

    Checker checker = {
        ast,
        malloc(sizeof(TypeId) * 1024), 0,
        malloc(sizeof(Node) * 1024), 0,
    };

    switch (first->kind) {
        case NodeKind_Invalid:
        case NodeKind_Literal: {
            fprintf(stderr, "Unknown node kind: '%d'\n", first->kind);
            free(checker.types);
            free(checker.decls);
            return (TypedAst) { NULL, NULL, 0 };
        }
        case NodeKind_Binary: {
            if (type_check_binary(&checker, first->binary) == 0) {
                fprintf(stderr, "Type error in binary!\n");
                return (TypedAst) { NULL, NULL, 0 };
            }
            return (TypedAst) { ast.nodes, checker.types, first };
        }
        case NodeKind_VarDecl: {
            type_check_var_decl(&checker, first->var_decl);
            return (TypedAst) { ast.nodes, checker.types, first };
        }
    }
}

