#include "checker.h"
#include "error.h"
#include "memory.h"
#include "../parser/visitor.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define TYPE_ERROR (TypeId)(-1)
#define TYPE_NONE  (TypeId)(-2)
#define TYPE_DECL  (TypeId)(-3)
#define TYPE_TYPE  (TypeId)(-4)


void typed_ast_free(TypedAst ast) {
    grammar_tree_free(ast.tree);
    Block* block = ast.block;
    for (int i = 0; i < ast.count; ++i) {
        dealloc(block[i].locals);
    }
    dealloc(ast.block);
    dealloc(ast.types);
    dealloc(ast.type_info);
}


/* ---------------------------- CHECKER IMPL -------------------------------- */
typedef struct Checker {
    Visitor visitor;
    Logger* logger;
    GrammarTree tree;

    Block* blocks;
    size_t block_count;

    Block* current;
    NodeFunDecl* current_function;

    TypeInfo* type_info;
    TypeId type_info_count;

    TypeId* types;
    TypeId  type_count;
} Checker;

static void checker_free(Checker* checker) {
    for (size_t i = 0; i < checker->block_count; ++i) {
        dealloc(checker->blocks[i].locals);
    }
    dealloc(checker->blocks);
    dealloc(checker->type_info);
    dealloc(checker->types);
    grammar_tree_free(checker->tree);
}

static TypedAst checker_to_ast(Checker* checker) {
    return (TypedAst) {
        checker->tree,
        checker->blocks,
        (i64) checker->tree.block_count + 1,
        checker->types,
        checker->type_count,
        checker->type_info,
        checker->type_info_count
    };
}

static Block* push_block(Checker* checker, const NodeBlock* block) {
    Block* current = checker->current;
    Block* x = checker->blocks + block->id;
    if (x->locals == NULL) {
        x->locals = (Local *) alloc(1024 * sizeof(Local));
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

static TypeId add_type(Checker* checker, Node* node, TypeId type) {
    NodeId id = node_id(&checker->tree, node);
    checker->types[id] = type;
    return type;
}

static TypeId add_type_info(Checker* checker, const char* name, int size, Node* decl) {
    checker->type_info[checker->type_info_count] = (TypeInfo) {
        .name = name,
        .size = size,
        .align = size,  // TODO: Fix this.
        .decl = decl,
    };
    return checker->type_info_count++;
}

Local* find_local(Checker* checker, const char* name) {
    Block* current = checker->current;
    while (current != NULL) {
        for (int i = 0; i < current->count; ++i) {
            Local* local = current->locals + i;
            assert(
                (local->decl->kind == NodeKind_VarDecl || local->decl->kind == NodeKind_FunDecl || local->decl->kind == NodeKind_FunParam || local->decl->kind == NodeKind_Struct)
                && "Invalid node kind"
            );

            switch (local->decl->kind) {
                case NodeKind_VarDecl:
                    if (local->decl->var_decl.name == name) {
                        return local;
                    }
                    break;
                case NodeKind_FunDecl:
                    if (local->decl->fun_decl.name == name)
                        return local;
                    break;
                case NodeKind_FunParam:
                    if (local->decl->fun_param.name == name)
                        return local;
                    break;
                case NodeKind_Struct:
                    if (local->decl->struct_decl.name == name)
                        return local;
                    break;
                default:
                    assert(0 && "Invalid node kind");
            }
        }
        current = current->parent == -1 ? NULL : checker->blocks + current->parent;
    }
    return NULL;
}

static void report_undeclared_identifier(Checker* checker, const char* name, const Node* node) {
    error(checker->logger, STR_FMT "\n    Undeclared identifier: '%s'\n", STR_ARG(checker->tree.tokens.name), name);
    int start = (int) checker->tree.tokens.source_offsets[node->base.start];
    int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
    const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

    point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
}

static void report_binary_op_mismatch(Checker* checker, const NodeBinary* binary, TypeId left, TypeId right) {
    error(checker->logger, STR_FMT "\n    Operator '%s' is not supported between '%s' and '%s'\n", STR_ARG(checker->tree.tokens.name), binary_op_repr(binary->op), literal_type_repr(left), literal_type_repr(right));
    int start = (int) checker->tree.tokens.source_offsets[binary->base.start];
    int end   = (int) checker->tree.tokens.source_offsets[binary->base.end];
    const char* repr = lexer_repr_of(checker->tree.tokens, binary->base.end);

    point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
}

static void report_type_expectation(Checker* checker, const char* prefix, const Node* node, TypeId expected, TypeId got) {
    expected = (expected == (TypeId) -1) ? LiteralType_Void : expected;
    got = (got == (TypeId) -1) ? LiteralType_Void : got;
    error(checker->logger, STR_FMT "\n    %s. Expected '%s', got '%s'\n", STR_ARG(checker->tree.tokens.name), prefix, literal_type_repr(expected), literal_type_repr(got));
    int start = (int) checker->tree.tokens.source_offsets[node->base.start];
    int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
    const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

    point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
}


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static TypeId resolve_type(Checker* checker, const Node* node) {
    assert(node->kind == NodeKind_Type && "Expected a type node");
    for (TypeId i = 0; i < checker->type_info_count; ++i) {
        if (checker->type_info[i].name == node->type.name)
            return (TypeId) i;
    }
    assert(0 && "Type not found");
}


static TypeId type_check_literal(Checker* checker, const NodeLiteral* node) {
    return add_type(checker, (Node*) node, node->type);
}

static TypeId type_check_identifier(Checker* checker, const NodeIdentifier* node) {
    Local* local = find_local(checker, node->name);
    if (local)
        return add_type(checker, (Node*) node, local->type);

    report_undeclared_identifier(checker, node->name, (Node*) node);
    return add_type(checker, (Node*) node, TYPE_ERROR);
}

static TypeId type_check_unary(Checker* checker, const NodeUnary* node) {
    TypeId expr = (TypeId) visit(checker, node->expr);
    if (expr == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    if (unary_op_is_logical(node->op)) {
        if (expr != LiteralType_Boolean) {
            report_type_expectation(checker, "Unary operator type mismatch", (Node*) node->expr, LiteralType_Boolean, expr);
            return add_type(checker, (Node*) node, TYPE_ERROR);
        }
        return LiteralType_Boolean;
    } else if (unary_op_is_arithmetic(node->op)) {
        if (expr != LiteralType_Integer) {
            report_type_expectation(checker, "Unary operator type mismatch", (Node*) node->expr, LiteralType_Integer, expr);
            return add_type(checker, (Node*) node, TYPE_ERROR);
        }
        return add_type(checker, (Node*) node, LiteralType_Integer);
    } else {
        error(checker->logger, STR_FMT "\n    Unary operator '%s' is not supported\n", STR_ARG(checker->tree.tokens.name), unary_op_repr(node->op));
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
        const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

        point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }
}

static TypeId type_check_binary(Checker* checker, const NodeBinary* node) {
    TypeId left = (TypeId) visit(checker, node->left);
    if (left == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    TypeId right = (TypeId) visit(checker, node->right);
    if (right == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    if (left != right) {
        report_binary_op_mismatch(checker, node, left, right);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    if (binary_op_is_relational(node->op) || binary_op_is_logical(node->op))
        return add_type(checker, (Node*) node, LiteralType_Boolean);
    else
        return add_type(checker, (Node*) node, left);
}

static TypeId type_check_call(Checker* checker, const NodeCall* node) {
    if (strcmp(node->name, "print") == 0) {
        for (i32 i = 0; i < node->count; ++i) {
            Node* arg = node->args[i];
            if ((TypeId) visit(checker, arg) == TYPE_ERROR)
                return add_type(checker, (Node*) node, TYPE_ERROR);
        }
        return add_type(checker, (Node*) node, LiteralType_Void);
    }

    Local* local = find_local(checker, node->name);
    if (local == NULL) {
        report_undeclared_identifier(checker, node->name, (Node*) node);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    if (local->decl->kind != NodeKind_FunDecl) {
        error(checker->logger, STR_FMT "\n    '%s' is not a function\n", STR_ARG(checker->tree.tokens.name), node->name);
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
        const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

        point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    NodeFunDecl* fun_decl = &local->decl->fun_decl;
    if (fun_decl->param_count != node->count) {
        error(checker->logger, STR_FMT "\n    Function '%s' requires %d arguments, got %d\n", STR_ARG(checker->tree.tokens.name), node->name, fun_decl->param_count, node->count);
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
        const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

        point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    for (i32 i = 0; i < fun_decl->param_count; ++i) {
        Node* arg = node->args[i];
        TypeId type = (TypeId) visit(checker, arg);
        if (type == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);

        TypeId param_type = resolve_type(checker, fun_decl->params[i]->type);
        if (type != param_type) {
            report_type_expectation(checker, "Argument type mismatch", arg, param_type, type);
            return add_type(checker, (Node*) node, TYPE_ERROR);
        }
    }

    if (fun_decl->return_type == NULL)
        return add_type(checker, (Node*) node, LiteralType_Void);
    else
        return add_type(checker, (Node*) node, resolve_type(checker, fun_decl->return_type));
}

static TypeId type_check_access(Checker* checker, const NodeAccess* node) {
    // TODO: Temporary.
    assert(node->right->kind == NodeKind_Identifier && "Access right must be an identifier");

    TypeId left = (TypeId) visit(checker, node->left);
    if (left == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    TypeInfo type = checker->type_info[left];
    if (type.decl->kind != NodeKind_Struct) {
        error(checker->logger, STR_FMT "\n    '%s' is not a struct\n", STR_ARG(checker->tree.tokens.name), type.name);
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
        const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

        point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    NodeStruct* struct_ = (NodeStruct*) type.decl;
    for (int i = 0; i < struct_->count; ++i) {
        NodeStructField* field = (NodeStructField*) struct_->nodes[i];
        if (field->name == node->right->identifier.name) {
            return add_type(checker, (Node*) node, resolve_type(checker, field->type));
        }
    }

    error(checker->logger, STR_FMT "\n    Struct '%s' does not have a field named '%s'\n", STR_ARG(checker->tree.tokens.name), struct_->name, node->right->identifier.name);
    int start = (int) checker->tree.tokens.source_offsets[node->base.start];
    int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
    const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);
    point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
    return add_type(checker, (Node*) node, TYPE_ERROR);
}

static TypeId type_check_var_decl(Checker* checker, const NodeVarDecl* node) {
    Local* local = find_local(checker, node->name);
    if (local) {
        error(checker->logger, STR_FMT "\n    Variable '%s' already declared\n", STR_ARG(checker->tree.tokens.name), node->name);
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
        const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

        point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    TypeId expr = (TypeId) visit(checker, node->expression);
    if (expr == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
        .type = expr,
        .decl = (Node*) node
    };
    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_type(Checker* checker, const NodeType* node) {
    for (TypeId i = 0; i < checker->type_info_count; ++i) {
        if (checker->type_info[i].name == node->name)
            return add_type(checker, (Node*) node, (TypeId) i);
    }

    report_undeclared_identifier(checker, node->name, (Node*) node);
    return add_type(checker, (Node*) node, TYPE_ERROR);
}

static TypeId type_check_assign(Checker* checker, const NodeAssign* node) {
    TypeId expr = (TypeId) visit(checker, node->expression);
    if (expr == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    Local* local = find_local(checker, node->name);
    if (local == NULL) {
        report_undeclared_identifier(checker, node->name, (Node*) node);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    if (local->type != expr) {
        report_type_expectation(checker, "Assignment type mismatch", node->expression, local->type, expr);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    return add_type(checker, (Node*) node, local->type);
}

static TypeId type_check_block(Checker* checker, const NodeBlock* node) {
    Block* parent = push_block(checker, node);
    for (i32 i = 0; i < node->count; ++i) {
        Node* stmt = node->nodes[i];
        if ((TypeId) visit(checker, stmt) == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);
    }
    restore_block(checker, parent);

    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_fun_param(Checker* checker, const NodeFunParam* node) {
    assert(node->expression == NULL && "Function parameters cannot have default values for now");

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
        .type = (TypeId) visit(checker, node->type),
        .decl = (Node*) node
    };
    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_fun_body(Checker* checker, const NodeFunBody* node) {
    Block* parent = push_block(checker, (const NodeBlock *) node);
    for (i32 i = 0; i < node->count; ++i) {
        Node* stmt = node->nodes[i];
        if ((TypeId) visit(checker, stmt) == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);
    }
    restore_block(checker, parent);
    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_fun_decl(Checker* checker, const NodeFunDecl* node) {
    Local* local = find_local(checker, node->name);
    if (local != NULL) {
        error(checker->logger, STR_FMT "\n    Function '%s' already declared\n", STR_ARG(checker->tree.tokens.name), node->name);
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->body->base.start];

        point_to_error(checker->logger, checker->tree.tokens.source, start, end);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    // Add parameters to the symbol table at the beginning of the function.
    Block* block = push_block(checker, (const NodeBlock*) node->body);
    for (i32 i = 0; i < node->param_count; ++i) {
        NodeFunParam* param = node->params[i];
        if ((TypeId)visit(checker, (Node*) param) == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);
    }
    restore_block(checker, block);

    NodeFunDecl* current_function = checker->current_function;
    checker->current_function = (NodeFunDecl*) node;
    if ((TypeId)visit(checker, (Node*)node->body) == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);
    checker->current_function = current_function;

    Block* current = checker->current;
    current->locals[current->count++] = (Local) {
            .type = node->return_type == NULL ? -1 : resolve_type(checker, node->return_type),
            .decl = (Node*) node
    };
    add_type_info(checker, node->name, 8, (Node *) node);

    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_return_stmt(Checker* checker, const NodeReturn* node) {
    if (checker->current_function == NULL) {
        error(checker->logger, STR_FMT "\n    Return statement outside of function\n", STR_ARG(checker->tree.tokens.name));
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) start + (int) strlen("return");

        point_to_error(checker->logger, checker->tree.tokens.source, start, end);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    TypeId expr = (TypeId) visit(checker, node->expression);
    if (expr == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    if (checker->current_function->return_type == NULL) {
        report_type_expectation(checker, "Return type mismatch", node->expression, 0, expr);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    TypeId return_type = resolve_type(checker, checker->current_function->return_type);
    if (expr != return_type) {
        report_type_expectation(checker, "Return type mismatch", node->expression, return_type, expr);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_if_stmt(Checker* checker, const NodeIf* node) {
    TypeId condition = (TypeId) visit(checker, node->condition);
    if (condition == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    if (condition != LiteralType_Boolean) {
        report_type_expectation(checker, "Condition of 'if' statement must be a boolean", (Node*) node->condition, LiteralType_Boolean, condition);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    if ((TypeId)visit(checker, (Node*) node->then_block) == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    if (node->else_block != NULL && (TypeId)visit(checker, (Node*) node->else_block) == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_while_stmt(Checker* checker, const NodeWhile* node) {
    TypeId condition = (TypeId) visit(checker, node->condition);
    if (condition == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    if (condition != LiteralType_Boolean) {
        report_type_expectation(checker, "Condition of 'while' statement must be a boolean", (Node*) node->condition, LiteralType_Boolean, condition);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    if ((TypeId)visit(checker, (Node*) node->then_block) == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    if (node->else_block != NULL && (TypeId)visit(checker, (Node*) node->then_block) == TYPE_ERROR)
        return add_type(checker, (Node*) node, TYPE_ERROR);

    return add_type(checker, (Node*) node, TYPE_NONE);
}

static TypeId type_check_init_arg(Checker* checker, const NodeInitArg* node) {
    TypeId expr = (TypeId) visit(checker, node->expr);
    return add_type(checker, (Node*) node, expr);
}

static TypeId type_check_init(Checker* checker, const NodeInit* node) {
    Local* decl = find_local(checker, node->name);
    if (!decl) {
        report_undeclared_identifier(checker, node->name, (Node*) node);
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    if (decl->decl->kind != NodeKind_Struct) {
        error(checker->logger, STR_FMT "\n    '%s' is not a struct\n", STR_ARG(checker->tree.tokens.name), node->name);
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
        const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

        point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    NodeStruct* struct_ = (NodeStruct*) decl->decl;

    // NOTE(ted): Temporary for now.
    if (node->count != struct_->count) {
        error(checker->logger, STR_FMT "\n    Struct '%s' requires %d arguments, got %d\n", STR_ARG(checker->tree.tokens.name), node->name, struct_->count, node->count);
        int start = (int) checker->tree.tokens.source_offsets[node->base.start];
        int end   = (int) checker->tree.tokens.source_offsets[node->base.end];
        const char* repr = lexer_repr_of(checker->tree.tokens, node->base.end);

        point_to_error(checker->logger, checker->tree.tokens.source, start, end + (int)strlen(repr));
        return add_type(checker, (Node*) node, TYPE_ERROR);
    }


    for (i32 i = 0; i < node->count; ++i) {
        NodeInitArg* arg = node->args[i];
        NodeStructField* field = (NodeStructField*) struct_->nodes[i];

        TypeId field_type = (TypeId) visit(checker, (Node*) field->type);
        TypeId arg_type = (TypeId) visit(checker, (Node*)arg->expr);
        if (arg_type != field_type) {
            report_type_expectation(checker, "Initialization type mismatch", (Node*) arg, field_type, arg_type);
            return add_type(checker, (Node*) node, TYPE_ERROR);
        }
    }

    return add_type(checker, (Node*) node, decl->type);
}

static TypeId type_check_struct_field(Checker* checker, const NodeStructField* node) {
    TypeId type = resolve_type(checker, node->type);

    if (node->expr) {
        TypeId expr_type = (TypeId) visit(checker, node->expr);
        if (expr_type == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);
        if (expr_type != type) {
            report_type_expectation(checker, "Struct field type mismatch", (Node*) node->expr, type, expr_type);
            return add_type(checker, (Node*) node, TYPE_ERROR);
        }
    }

    return add_type(checker, (Node*) node, type);
}

static TypeId type_check_struct_decl(Checker* checker, const NodeStruct* node) {
    Block* parent = push_block(checker, (const NodeBlock *) node);

    int size = 0;
    for (i32 i = 0; i < node->count; ++i) {
        Node* field = node->nodes[i];
        TypeId type = (TypeId) visit(checker, field);
        if (type == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);
        size += checker->type_info[type].size;
    }
    restore_block(checker, parent);

    Block* current = checker->current;
    TypeId type = add_type_info(checker, node->name, size, (Node *) node);
    current->locals[current->count++] = (Local) {
            .type = type,
            .decl = (Node*) node
    };

    return add_type(checker, (Node*) node, TYPE_NONE);
}


static TypeId type_check_module(Checker* checker, const NodeModule* node) {
    NodeBlock block = { .id = 0, .parent=-1 };
    Block* parent = push_block(checker, &block);
    for (i32 i = 0; i < node->decl_count; ++i) {
        Node* node_ = node->decls[i];
        if ((TypeId)visit(checker, node_) == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);
    }

    for (i32 i = 0; i < node->stmt_count; ++i) {
        Node* node_ = node->stmts[i];
        if ((TypeId) visit(checker, node_) == TYPE_ERROR)
            return add_type(checker, (Node*) node, TYPE_ERROR);
    }
    restore_block(checker, parent);

    return add_type(checker, (Node*) node, TYPE_NONE);
}


void add_builtin_types(Checker* checker) {
    size_t i = 0;
#define X(upper, lower, repr, size) add_type_info(checker, (const char*) i++, size, NULL);
    ALL_LITERAL_TYPES(X)
#undef X
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
        .tree = ast,
        .blocks = (Block*) memset(alloc((ast.block_count + 1) * sizeof(Block)), 0, (ast.block_count + 1) * sizeof(Block)),
        .block_count = ast.block_count + 1,
        .current = NULL,
        .current_function = NULL,
        .type_info = (TypeInfo*) alloc(1024 * sizeof(TypeInfo)),
        .type_info_count = 0,
        .types = (TypeId*) alloc(1024 * sizeof(TypeId)),
        .type_count = 1024,
    };

    add_builtin_types(&checker);
    TypeId type = (TypeId) visit(&checker.visitor, node);

    if (type == TYPE_ERROR) {
        checker_free(&checker);
        return (TypedAst) { 0 };
    }

    return checker_to_ast(&checker);
}


