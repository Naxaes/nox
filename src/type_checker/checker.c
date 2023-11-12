#include "checker.h"
#include "error.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


void typed_ast_free(TypedAst ast) {
    free(ast.nodes);
    Block* block = ast.block;
    while (block != NULL) {
        free(block->locals);
        block++;
    }
    free(ast.block);
}


/* ---------------------------- CHECKER IMPL -------------------------------- */
typedef struct {
    UntypedAst ast;

    Block* blocks;
    size_t block_count;

    Block* current;
} Checker;

static void checker_free(Checker* checker) {
    for (size_t i = 0; i < checker->block_count; ++i) {
        free(checker->blocks[i].locals);
    }
    free(checker->blocks);
    grammar_tree_free(checker->ast);
}

static TypedAst checker_to_ast(Checker* checker) {
    return (TypedAst) {
        checker->ast.nodes,
        checker->ast.views,
        checker->ast.start,
        checker->blocks,
    };
}

static size_t push_block(Checker* checker) {
    size_t parent = -1;
    if (checker->current != NULL) {
        parent = checker->current - checker->blocks;
    }
    checker->current = checker->blocks + checker->block_count++;
    checker->current->parent = parent;
    checker->current->locals = (Local *) malloc(1024 * sizeof(Local));
    checker->current->count = 0;
    return parent;
}

static void restore_block(Checker* checker, size_t id) {
    checker->current = checker->blocks + id;
}

static Local* find_local(Checker* checker, const char* name) {
    Block* current = checker->current;
    while (current != NULL) {
        for (size_t i = 0; i < current->count; ++i) {
            Local* local = current->locals + i;
            assert((local->decl->kind == NodeKind_VarDecl || local->decl->kind == NodeKind_FunDecl || local->decl->kind == NodeKind_FunParam) && "Invalid node kind");
            if (local->decl->var_decl.name == name) {
                return local;
            }
        }
        current = current->parent == (size_t)-1 ? NULL : checker->blocks + current->parent;
    }
    return NULL;
}

static void report_undeclared_identifier(Checker* checker, const char* name, Node* node) {
    fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Undeclared identifier: '%s'\n", STR_ARG(checker->ast.tokens.name), name);
    int start = (int) checker->ast.tokens.source_offsets[node->base.start];
    int end   = (int) checker->ast.tokens.source_offsets[node->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, node->base.end);

    point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
}

static void report_binary_op_mismatch(Checker* checker, NodeBinary* binary, TypeId left, TypeId right) {
    fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Operator '%s' is not supported between '%s' and '%s'\n", STR_ARG(checker->ast.tokens.name), binary_op_repr(binary->op), literal_type_repr(left), literal_type_repr(right));
    int start = (int) checker->ast.tokens.source_offsets[binary->base.start];
    int end   = (int) checker->ast.tokens.source_offsets[binary->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, binary->base.end);

    point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
}

static void report_type_expectation(Checker* checker, const char* prefix, Node* node, TypeId expected, TypeId got) {
    fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    %s. Expected '%s', got '%s'\n", STR_ARG(checker->ast.tokens.name), prefix, literal_type_repr(expected), literal_type_repr(got));
    int start = (int) checker->ast.tokens.source_offsets[node->base.start];
    int end   = (int) checker->ast.tokens.source_offsets[node->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, node->base.end);

    point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
}


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static TypeId type_check_expression(Checker* checker, Node* node);
static TypeId type_check_statement(Checker* checker, Node* node);


static TypeId type_check_literal(Checker* checker, NodeLiteral* literal) {
    (void)checker;
    return literal->type;
}

static TypeId type_check_identifier(Checker* checker, NodeIdentifier* identifier) {
    Local* local = find_local(checker, identifier->name);
    if (local)
        return local->type;

    report_undeclared_identifier(checker, identifier->name, (Node*) identifier);
    return 0;
}

static TypeId type_check_binary(Checker* checker, NodeBinary* binary) {
    TypeId left  = type_check_expression(checker, binary->left);
    if (left == 0)
        return 0;

    TypeId right = type_check_expression(checker, binary->right);
    if (right == 0)
        return 0;

    if (left != right) {
        report_binary_op_mismatch(checker, binary, left, right);
        return 0;
    }

    if (binary_op_is_relational(binary->op))
        return LiteralType_Boolean;
    else
        return left;
}

static TypeId type_check_call(Checker* checker, NodeCall* call) {
    if (strcmp(call->name, "print") == 0)
        return -1;

    Local* local = find_local(checker, call->name);
    if (local == NULL) {
        report_undeclared_identifier(checker, call->name, (Node*) call);
        return 0;
    }
    if (local->decl->kind != NodeKind_FunDecl) {
        fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    '%s' is not a function\n", STR_ARG(checker->ast.tokens.name), call->name);
        int start = (int) checker->ast.tokens.source_offsets[call->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[call->base.end];
        const char* repr = lexer_repr_of(checker->ast.tokens, call->base.end);

        point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
        return 0;
    }

    NodeFunDecl* fun_decl = &local->decl->fun_decl;
    if (fun_decl->params != NULL && call->args != NULL) {
        NodeFunParam* param = NULL;
        NodeFunParam** params = fun_decl->params;
        Node** args = call->args;
        while ((param = *params++) != NULL) {
            Node* arg = *args++;
            if (arg == NULL) {
                fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Too few arguments for function '%s'\n", STR_ARG(checker->ast.tokens.name), call->name);
                int start = (int) checker->ast.tokens.source_offsets[call->base.start];
                int end   = (int) checker->ast.tokens.source_offsets[call->base.end];
                const char* repr = lexer_repr_of(checker->ast.tokens, call->base.end);

                point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
                return 0;
            }
            TypeId type = type_check_expression(checker, arg);
            if (type == 0)
                return 0;
            if (type != param->type) {
                report_type_expectation(checker, "Argument type mismatch", (Node*) param, param->type, type);
                return 0;
            }
        }
        if (*args != NULL) {
            fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Too many arguments for function '%s'\n", STR_ARG(checker->ast.tokens.name), call->name);
            int start = (int) checker->ast.tokens.source_offsets[call->base.start];
            int end   = (int) checker->ast.tokens.source_offsets[call->base.end];
            const char* repr = lexer_repr_of(checker->ast.tokens, call->base.end);

            point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
            return 0;
        }
    } else if (call->args != NULL) {
        fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Function '%s' does not take any arguments\n", STR_ARG(checker->ast.tokens.name), call->name);
        int start = (int) checker->ast.tokens.source_offsets[call->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[call->base.end];
        const char* repr = lexer_repr_of(checker->ast.tokens, call->base.end);

        point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
        return 0;
    } else if (fun_decl->params != NULL) {
        fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Function '%s' requires arguments\n", STR_ARG(checker->ast.tokens.name), call->name);
        int start = (int) checker->ast.tokens.source_offsets[call->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[call->base.end];
        const char* repr = lexer_repr_of(checker->ast.tokens, call->base.end);

        point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
        return 0;
    }

    return -1;
}

static TypeId type_check_var_decl(Checker* checker, NodeVarDecl* var_decl) {
    TypeId expr = type_check_expression(checker, var_decl->expression);
    if (expr == 0)
        return 0;

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
        .type = expr,
        .decl = (Node*) var_decl
    };
    return -1;
}

static TypeId type_check_assignment(Checker* checker, NodeAssign* assign) {
    TypeId expr = type_check_expression(checker, assign->expression);
    if (expr == 0)
        return 0;

    Local* local = find_local(checker, assign->name);
    if (local)
        return local->type;

    report_undeclared_identifier(checker, assign->name, (Node*) assign);
    return 0;
}

static TypeId type_check_block(Checker* checker, NodeBlock* block, int new_block) {
    if (block->nodes != NULL) {
        size_t parent =  -1;
        if (new_block)
            parent = push_block(checker);

        Node* node = NULL;
        Node** nodes = block->nodes;
        while ((node = *(nodes++)) != NULL) {
            if (type_check_statement(checker, node) == 0)
                return 0;
        }

        if (new_block)
            restore_block(checker, parent);
    }
    return -1;
}

static TypeId type_check_fun_param(Checker* checker, NodeFunParam* fun_param) {
    assert(fun_param->expression == NULL && "Function parameters cannot have default values for now");

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
            .type = fun_param->type,
            .decl = (Node*) fun_param
    };
    return -1;
}

static TypeId type_check_fun_decl(Checker* checker, NodeFunDecl* fun_decl) {
    size_t parent = push_block(checker);

    if (fun_decl->params != NULL) {
        NodeFunParam* param = NULL;
        NodeFunParam** params = fun_decl->params;
        while ((param = *params++) != NULL) {
            if (type_check_fun_param(checker, param) == 0)
                return 0;
        }
    }

    if (type_check_block(checker, fun_decl->block, 0) == 0)
        return 0;

    restore_block(checker, parent);
    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
            .type = fun_decl->return_type,
            .decl = (Node*) fun_decl
    };

    return -1;
}

static TypeId type_check_if_stmt(Checker* checker, NodeIf* if_stmt) {
    TypeId condition = type_check_expression(checker, if_stmt->condition);
    if (condition == 0)
        return 0;

    if (condition != LiteralType_Boolean) {
        report_type_expectation(checker, "Condition of 'if' statement must be a boolean", (Node*) if_stmt->condition, LiteralType_Boolean, condition);
        return 0;
    }

    if (type_check_block(checker, if_stmt->then_block, 1) == 0)
        return 0;

    if (if_stmt->else_block != NULL && type_check_block(checker, if_stmt->then_block, 1) == 0)
        return 0;

    return -1;
}

static TypeId type_check_while_stmt(Checker* checker, NodeWhile* while_stmt) {
    TypeId condition = type_check_expression(checker, while_stmt->condition);
    if (condition == 0)
        return 0;

    if (condition != LiteralType_Boolean) {
        report_type_expectation(checker, "Condition of 'while' statement must be a boolean", (Node*) while_stmt->condition, LiteralType_Boolean, condition);
        return 0;
    }

    if (type_check_block(checker, while_stmt->then_block, 1) == 0)
        return 0;

    if (while_stmt->else_block != NULL && type_check_block(checker, while_stmt->then_block, 1) == 0)
        return 0;

    return -1;
}

static TypeId type_check_expression(Checker* checker, Node* node) {
    assert(node_is_expression(node) && "Not an expression");
    switch (node->kind) {
        case NodeKind_Literal:
            return type_check_literal(checker, &node->literal);
        case NodeKind_Identifier:
            return type_check_identifier(checker, &node->identifier);
        case NodeKind_Binary:
            return type_check_binary(checker, &node->binary);
        case NodeKind_Call:
            return type_check_call(checker, &node->call);
        default:
            assert(0 && "not implemented");
    }
}

static TypeId type_check_statement(Checker* checker, Node* node) {
    switch (node->kind) {
        case NodeKind_Literal:
        case NodeKind_Identifier:
        case NodeKind_Binary:
        case NodeKind_Call:
            return type_check_expression(checker, node);
        case NodeKind_Assign:
            return type_check_assignment(checker, &node->assign);
        case NodeKind_VarDecl:
            return type_check_var_decl(checker, &node->var_decl);
        case NodeKind_If:
            return type_check_if_stmt(checker, &node->if_stmt);
        case NodeKind_While:
            return type_check_while_stmt(checker, &node->while_stmt);
        case NodeKind_Block:
            return type_check_block(checker, &node->block, 1);
        case NodeKind_FunDecl:
            return type_check_fun_decl(checker, &node->fun_decl);
        default:
            assert(0 && "not implemented");
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

    TypeId type = type_check_statement(&checker, node);
    if (type == 0) {
        checker_free(&checker);
        return (TypedAst) { NULL, NULL, NULL, NULL };
    }

    return checker_to_ast(&checker);
}


