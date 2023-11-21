#include "checker.h"
#include "error.h"
#include "memory.h"
#include "../parser/visitor.h"

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
    Visitor visitor;
    Logger* logger;
    GrammarTree ast;

    Block* blocks;
    size_t block_count;

    Block* current;
    NodeFunDecl* current_function;
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

static Block* push_block(Checker* checker, const NodeBlock* block) {
    Block* current = checker->current;
    Block* x = checker->blocks + block->id;
    if (x->locals == NULL) {
        x->locals = (Local *) malloc(1024 * sizeof(Local));
        x->count = 0;
        x->parent = block->parent;
    }
    x->parent_count = (current == NULL) ? 0 : (i32)(current->count + current->parent_count);
    checker->current = x;
    return current;
}

static void restore_block(Checker* checker, Block* block) {
    checker->current = block;
}

static Local* find_local(Checker* checker, const char* name) {
    Block* current = checker->current;
    while (current != NULL) {
        for (int i = 0; i < current->count; ++i) {
            Local* local = current->locals + i;
            assert((local->decl->kind == NodeKind_VarDecl || local->decl->kind == NodeKind_FunDecl || local->decl->kind == NodeKind_FunParam) && "Invalid node kind");
            if (local->decl->var_decl.name == name) {
                return local;
            }
        }
        current = current->parent == -1 ? NULL : checker->blocks + current->parent;
    }
    return NULL;
}

static void report_undeclared_identifier(Checker* checker, const char* name, const Node* node) {
    error(checker->logger, STR_FMT "\n    Undeclared identifier: '%s'\n", STR_ARG(checker->ast.tokens.name), name);
    int start = (int) checker->ast.tokens.source_offsets[node->base.start];
    int end   = (int) checker->ast.tokens.source_offsets[node->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, node->base.end);

    point_to_error(checker->logger, checker->ast.tokens.source, start, end + (int)strlen(repr));
}

static void report_binary_op_mismatch(Checker* checker, const NodeBinary* binary, TypeId left, TypeId right) {
    error(checker->logger, STR_FMT "\n    Operator '%s' is not supported between '%s' and '%s'\n", STR_ARG(checker->ast.tokens.name), binary_op_repr(binary->op), literal_type_repr(left), literal_type_repr(right));
    int start = (int) checker->ast.tokens.source_offsets[binary->base.start];
    int end   = (int) checker->ast.tokens.source_offsets[binary->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, binary->base.end);

    point_to_error(checker->logger, checker->ast.tokens.source, start, end + (int)strlen(repr));
}

static void report_type_expectation(Checker* checker, const char* prefix, const Node* node, TypeId expected, TypeId got) {
    error(checker->logger, STR_FMT "\n    %s. Expected '%s', got '%s'\n", STR_ARG(checker->ast.tokens.name), prefix, literal_type_repr(expected), literal_type_repr(got));
    int start = (int) checker->ast.tokens.source_offsets[node->base.start];
    int end   = (int) checker->ast.tokens.source_offsets[node->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, node->base.end);

    point_to_error(checker->logger, checker->ast.tokens.source, start, end + (int)strlen(repr));
}


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static TypeId type_check_literal(Checker* checker, const NodeLiteral* literal) {
    (void)checker;
    return literal->type;
}

static TypeId type_check_identifier(Checker* checker, const NodeIdentifier* identifier) {
    Local* local = find_local(checker, identifier->name);
    if (local)
        return local->type;

    report_undeclared_identifier(checker, identifier->name, (Node*) identifier);
    return 0;
}

static TypeId type_check_binary(Checker* checker, const NodeBinary* binary) {
    TypeId left = (TypeId) visit(checker, binary->left);
    if (left == 0)
        return 0;

    TypeId right = (TypeId) visit(checker, binary->right);
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

static TypeId type_check_call(Checker* checker, const NodeCall* call) {
    if (strcmp(call->name, "print") == 0)
        return -1;

    Local* local = find_local(checker, call->name);
    if (local == NULL) {
        report_undeclared_identifier(checker, call->name, (Node*) call);
        return 0;
    }

    if (local->decl->kind != NodeKind_FunDecl) {
        error(checker->logger, STR_FMT "\n    '%s' is not a function\n", STR_ARG(checker->ast.tokens.name), call->name);
        int start = (int) checker->ast.tokens.source_offsets[call->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[call->base.end];
        const char* repr = lexer_repr_of(checker->ast.tokens, call->base.end);

        point_to_error(checker->logger, checker->ast.tokens.source, start, end + (int)strlen(repr));
        return 0;
    }

    NodeFunDecl* fun_decl = &local->decl->fun_decl;
    if (fun_decl->param_count != call->count) {
        error(checker->logger, STR_FMT "\n    Function '%s' requires %d arguments, got %d\n", STR_ARG(checker->ast.tokens.name), call->name, fun_decl->param_count, call->count);
        int start = (int) checker->ast.tokens.source_offsets[call->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[call->base.end];
        const char* repr = lexer_repr_of(checker->ast.tokens, call->base.end);

        point_to_error(checker->logger, checker->ast.tokens.source, start, end + (int)strlen(repr));
        return 0;
    }

    for (i32 i = 0; i < fun_decl->param_count; ++i) {
        Node* arg = call->args[i];
        TypeId type = (TypeId) visit(checker, arg);
        if (type == 0)
            return 0;

        if (type != (TypeId) fun_decl->params[i]->type->type.name) {
            report_type_expectation(checker, "Argument type mismatch", arg, (TypeId) fun_decl->params[i]->type, type);
            return 0;
        }
    }

    if (fun_decl->return_type == NULL)
        return -1;
    else
        return (TypeId) fun_decl->return_type->type.name;
}

static TypeId type_check_var_decl(Checker* checker, const NodeVarDecl* var_decl) {
    Local* local = find_local(checker, var_decl->name);
    if (local) {
        error(checker->logger, STR_FMT "\n    Variable '%s' already declared\n", STR_ARG(checker->ast.tokens.name), var_decl->name);
        int start = (int) checker->ast.tokens.source_offsets[var_decl->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[var_decl->base.end];
        const char* repr = lexer_repr_of(checker->ast.tokens, var_decl->base.end);

        point_to_error(checker->logger, checker->ast.tokens.source, start, end + (int)strlen(repr));
        return 0;
    }

    TypeId expr = (TypeId) visit(checker, var_decl->expression);
    if (expr == 0)
        return 0;

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
        .type = expr,
        .decl = (Node*) var_decl
    };
    return -1;
}

static TypeId type_check_type(Checker* checker, const NodeType* node) {
    (void)checker;
    // TODO: Add to symbol table.
    return (TypeId) node->name;
}

static TypeId type_check_assign(Checker* checker, const NodeAssign* assign) {
    TypeId expr = (TypeId) visit(checker, assign->expression);
    if (expr == 0)
        return 0;

    Local* local = find_local(checker, assign->name);
    if (local)
        return local->type;

    report_undeclared_identifier(checker, assign->name, (Node*) assign);
    return 0;
}

static TypeId type_check_block(Checker* checker, const NodeBlock* node) {
    Block* parent = push_block(checker, node);
    for (i32 i = 0; i < node->count; ++i) {
        Node* stmt = node->nodes[i];
        if (visit(checker, stmt) == 0)
            return 0;
    }
    restore_block(checker, parent);

    return -1;
}

static TypeId type_check_fun_param(Checker* checker, const NodeFunParam* fun_param) {
    assert(fun_param->expression == NULL && "Function parameters cannot have default values for now");

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
        .type = (TypeId) visit(checker, fun_param->type),
        .decl = (Node*) fun_param
    };
    return -1;
}

static TypeId type_check_fun_body(Checker* checker, const NodeFunBody* node) {
    Block* parent = push_block(checker, (const NodeBlock *) node);
    for (i32 i = 0; i < node->count; ++i) {
        Node* stmt = node->nodes[i];
        if (visit(checker, stmt) == 0)
            return 0;
    }
    restore_block(checker, parent);
    return -1;
}

static TypeId type_check_fun_decl(Checker* checker, const NodeFunDecl* fun_decl) {
    Local* local = find_local(checker, fun_decl->name);
    if (local != NULL) {
        error(checker->logger, STR_FMT "\n    Function '%s' already declared\n", STR_ARG(checker->ast.tokens.name), fun_decl->name);
        int start = (int) checker->ast.tokens.source_offsets[fun_decl->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[fun_decl->body->base.start];

        point_to_error(checker->logger, checker->ast.tokens.source, start, end);
        return 0;
    }

    // Add parameters to the symbol table at the beginning of the function.
    Block* block = push_block(checker, (const NodeBlock*) fun_decl->body);
    for (i32 i = 0; i < fun_decl->param_count; ++i) {
        NodeFunParam* param = fun_decl->params[i];
        if (type_check_fun_param(checker, param) == 0)
            return 0;
    }
    restore_block(checker, block);

    NodeFunDecl* current_function = checker->current_function;
    checker->current_function = (NodeFunDecl*) fun_decl;
    if (type_check_fun_body(checker, fun_decl->body) == 0)
        return 0;
    checker->current_function = current_function;

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
            .type = (TypeId) fun_decl->return_type,
            .decl = (Node*) fun_decl
    };

    return -1;
}

static TypeId type_check_return_stmt(Checker* checker, const NodeReturn* return_stmt) {
    if (checker->current_function == NULL) {
        error(checker->logger, STR_FMT "\n    Return statement outside of function\n", STR_ARG(checker->ast.tokens.name));
        int start = (int) checker->ast.tokens.source_offsets[return_stmt->base.start];
        int end   = (int) checker->ast.tokens.source_offsets[return_stmt->base.end];

        point_to_error(checker->logger, checker->ast.tokens.source, start, end);
        return 0;
    }

    TypeId expr = (TypeId) visit(checker, return_stmt->expression);
    if (expr == 0)
        return 0;

    if (expr != (TypeId) checker->current_function->return_type->type.name) {
        report_type_expectation(checker, "Return type mismatch", return_stmt->expression, (TypeId) checker->current_function->return_type->type.name, expr);
        return 0;
    }

    return -1;
}

static TypeId type_check_if_stmt(Checker* checker, const NodeIf* if_stmt) {
    TypeId condition = (TypeId) visit(checker, if_stmt->condition);
    if (condition == 0)
        return 0;

    if (condition != LiteralType_Boolean) {
        report_type_expectation(checker, "Condition of 'if' statement must be a boolean", (Node*) if_stmt->condition, LiteralType_Boolean, condition);
        return 0;
    }

    if (type_check_block(checker, if_stmt->then_block) == 0)
        return 0;

    if (if_stmt->else_block != NULL && type_check_block(checker, if_stmt->then_block) == 0)
        return 0;

    return -1;
}

static TypeId type_check_while_stmt(Checker* checker, const NodeWhile* while_stmt) {
    TypeId condition = (TypeId) visit(checker, while_stmt->condition);
    if (condition == 0)
        return 0;

    if (condition != LiteralType_Boolean) {
        report_type_expectation(checker, "Condition of 'while' statement must be a boolean", (Node*) while_stmt->condition, LiteralType_Boolean, condition);
        return 0;
    }

    if (type_check_block(checker, while_stmt->then_block) == 0)
        return 0;

    if (while_stmt->else_block != NULL && type_check_block(checker, while_stmt->then_block) == 0)
        return 0;

    return -1;
}



static TypeId type_check_module(Checker* checker, const NodeModule* node) {
    NodeBlock block = { .id = 0, .parent=-1 };
    Block* parent = push_block(checker, &block);
    for (i32 i = 0; i < node->decl_count; ++i) {
        Node* node_ = node->decls[i];
        if (visit(checker, node_) == 0)
            return 0;
    }

    for (i32 i = 0; i < node->stmt_count; ++i) {
        Node* node_ = node->stmts[i];
        if (visit(checker, node_) == 0)
            return 0;
    }
    restore_block(checker, parent);

    return -1;
}


TypedAst type_check(GrammarTree ast, Logger* logger) {
    Node* node = ast.start;

    Visitor visitor = {
#define X(upper, lower, flags, body) .visit_##lower = (Visit##upper##Fn) type_check_##lower,
        ALL_NODES(X)
#undef X
    };

    Checker checker = {
        .visitor = visitor,
        .logger = logger,
        .ast = ast,
        .blocks = (Block*) alloc((ast.block_count + 1) * sizeof(Block)),
        .block_count = 0,
        .current = NULL,
        .current_function = NULL,
    };

    TypeId type = (TypeId) visit(&checker.visitor, node);

    if (type == 0) {
        checker_free(&checker);
        return (TypedAst) { NULL, NULL, NULL, NULL };
    }

    return checker_to_ast(&checker);
}


