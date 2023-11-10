#include "checker.h"
#include "error.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


/* ---------------------------- TYPES -------------------------------- */
static const char* type_repr_of(TypeId type) {
    switch (type) {
        case Literal_Boolean: return "bool";
        case Literal_Integer: return "int";
        case Literal_Real:    return "real";
        case Literal_String:  return "string";
        default:
            assert(0 && "Not implemented");
    }
    assert(0 && "Invalid type id");
    return NULL;
}

void typed_ast_free(TypedAst ast) {
    free(ast.nodes);
    Block* block = ast.blocks;
    while (block != NULL) {
        free(block->locals);
        block++;
    }
    free(ast.blocks);
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
        checker->blocks,
        checker->ast.start,
    };
}

static void push_block(Checker* checker) {
    size_t parent = -1;
    if (checker->current != NULL) {
        parent = checker->current - checker->blocks;
    }
    checker->current = checker->blocks + checker->block_count++;
    checker->current->parent = parent;
    checker->current->locals = (Local *) malloc(1024 * sizeof(Local));
    checker->current->count = 0;
}

static Local* find_local(Checker* checker, const char* name) {
    Block* current = checker->current;
    for (size_t i = 0; i < current->count; ++i) {
        Local* local = current->locals + i;
        assert(local->decl.kind == NodeKind_VarDecl && "Invalid node kind");
        if (local->decl.var_decl.name == name) {
            return local;
        }
    }
    return NULL;
}

static void report_undeclared_identifier(Checker* checker, const char* name, Node* node) {
    fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Undeclared identifier: '%s'\n", STR_ARG(checker->ast.tokens.name), name);
    int start = (int) checker->ast.tokens.indices[node->base.start];
    int end   = (int) checker->ast.tokens.indices[node->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, node->base.end);

    point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
}

static void report_binary_op_mismatch(Checker* checker, NodeBinary binary, TypeId left, TypeId right) {
    fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    Operator '%s' is not supported between '%s' and '%s'\n", STR_ARG(checker->ast.tokens.name), binary_op_repr(binary), type_repr_of(left), type_repr_of(right));
    int start = (int) checker->ast.tokens.indices[binary.base.start];
    int end   = (int) checker->ast.tokens.indices[binary.base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, binary.base.end);

    point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
}

static void report_type_expectation(Checker* checker, const char* prefix, Node* node, TypeId expected, TypeId got) {
    fprintf(stderr, "[Error] (Checker) " STR_FMT "\n    %s. Expected '%s', got '%s'\n", STR_ARG(checker->ast.tokens.name), prefix, type_repr_of(expected), type_repr_of(got));
    int start = (int) checker->ast.tokens.indices[node->base.start];
    int end   = (int) checker->ast.tokens.indices[node->base.end];
    const char* repr = lexer_repr_of(checker->ast.tokens, node->base.end);

    point_to_error(checker->ast.tokens.source, start, end + (int)strlen(repr));
}


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static TypeId type_check_expression(Checker* checker, Node* node);
static TypeId type_check_statement(Checker* checker, Node* node);


static TypeId type_check_literal(Checker* checker, NodeLiteral literal) {
    (void)checker;
    return literal.type;
}

static TypeId type_check_identifier(Checker* checker, NodeIdentifier identifier) {
    Local* local = find_local(checker, identifier.name);
    if (local)
        return local->type;

    report_undeclared_identifier(checker, identifier.name, (Node*) &identifier);
    return 0;
}

static TypeId type_check_binary(Checker* checker, NodeBinary binary) {
    TypeId left  = type_check_expression(checker, binary.left);
    if (left == 0)
        return 0;

    TypeId right = type_check_expression(checker, binary.right);
    if (right == 0)
        return 0;

    if (left != right) {
        report_binary_op_mismatch(checker, binary, left, right);
        return 0;
    }

    if (binary_op_is_comparison(binary))
        return Literal_Boolean;
    else
        return left;
}

static TypeId type_check_call(Checker* checker, NodeCall call) {
    (void)checker;
    (void)call;
    return -1;
}

static TypeId type_check_var_decl(Checker* checker, NodeVarDecl var_decl) {
    TypeId expr = type_check_expression(checker, var_decl.expression);
    if (expr == 0)
        return 0;

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
        .type = expr,
        .decl = (Node) { .var_decl = var_decl },
    };
    return -1;
}

static TypeId type_check_assignment(Checker* checker, NodeAssign assign) {
    TypeId expr = type_check_expression(checker, assign.expression);
    if (expr == 0)
        return 0;

    Local* local = find_local(checker, assign.name);
    if (local)
        return local->type;

    report_undeclared_identifier(checker, assign.name, (Node*) &assign);
    return 0;
}

static TypeId type_check_block(Checker* checker, NodeBlock block) {
    Node* node = NULL;
    while ((node = *block.nodes++) != NULL) {
        if (type_check_statement(checker, node) == 0)
            return 0;
    }
    return -1;
}

static TypeId type_check_if_stmt(Checker* checker, NodeIf if_stmt) {
    TypeId condition = type_check_expression(checker, if_stmt.condition);
    if (condition == 0)
        return 0;

    if (condition != Literal_Boolean) {
        report_type_expectation(checker, "Condition of 'if' statement must be a boolean", (Node*) if_stmt.condition, Literal_Boolean, condition);
        return 0;
    }

    if (type_check_block(checker, *if_stmt.then_block) == 0)
        return 0;

    if (if_stmt.else_block != NULL && type_check_block(checker, *if_stmt.then_block) == 0)
        return 0;

    return -1;
}

static TypeId type_check_while_stmt(Checker* checker, NodeWhile while_stmt) {
    TypeId condition = type_check_expression(checker, while_stmt.condition);
    if (condition == 0)
        return 0;

    if (condition != Literal_Boolean) {
        report_type_expectation(checker, "Condition of 'while' statement must be a boolean", (Node*) while_stmt.condition, Literal_Boolean, condition);
        return 0;
    }

    if (type_check_block(checker, *while_stmt.then_block) == 0)
        return 0;

    if (while_stmt.else_block != NULL && type_check_block(checker, *while_stmt.then_block) == 0)
        return 0;

    return -1;
}

static TypeId type_check_expression(Checker* checker, Node* node) {
    assert(node_is_expression(node) && "Not an expression");
    switch (node->kind) {
        case NodeKind_Literal:
            return type_check_literal(checker, node->literal);
        case NodeKind_Identifier:
            return type_check_identifier(checker, node->identifier);
        case NodeKind_Binary:
            return type_check_binary(checker, node->binary);
        case NodeKind_Call:
            return type_check_call(checker, node->call);
        default:
            assert(0 && "not implemented");
    }
}

static TypeId type_check_statement(Checker* checker, Node* node) {
    assert(node_is_statement(node) && "Not a statement");
    switch (node->kind) {
        case NodeKind_Literal:
        case NodeKind_Identifier:
        case NodeKind_Binary:
        case NodeKind_Call:
            return type_check_expression(checker, node);
        case NodeKind_Assign:
            return type_check_assignment(checker, node->assign);
        case NodeKind_VarDecl:
            return type_check_var_decl(checker, node->var_decl);
        case NodeKind_If:
            return type_check_if_stmt(checker, node->if_stmt);
        case NodeKind_While:
            return type_check_while_stmt(checker, node->while_stmt);
        case NodeKind_Block:
            return type_check_block(checker, node->block);
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

    push_block(&checker);

    TypeId type = type_check_statement(&checker, node);
    if (type == 0) {
        checker_free(&checker);
        return (TypedAst) { NULL, NULL, NULL };
    }

    return checker_to_ast(&checker);
}


