#include <stdio.h>
#include "checker.h"


int type_check_literal(UntypedAst ast, NodeLiteral literal) {
    return 1;
}


TypedAst type_check(UntypedAst ast) {
    Node first = ast.nodes[0];

    switch (first.kind) {
        case NodeKind_Invalid: {
            fprintf(stderr, "Unknown node kind: '%d'\n", first.kind);
            return (TypedAst) { ast.nodes };
        }
        case NodeKind_Literal: {
            if (!type_check_literal(ast, first.literal)) {
                fprintf(stderr, "Type error!\n");
            }
            return (TypedAst) { ast.nodes };
        } break;
    }
}

