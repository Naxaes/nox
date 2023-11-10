#include "tree_writer.h"

#include "error.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


typedef struct {
    UntypedAst* ast;
    FILE* output;
    const char* msg;
    int   indentation;
    int   last[24];
} TreeWriter;

static void print_indentation_(TreeWriter* writer, const int is_last) {
    if (writer->indentation == 0)
        return;


    if (writer->indentation == 1) {
        if (is_last) {
            fprintf(writer->output, "  └─ %-6s ", writer->msg);
        } else {
            fprintf(writer->output, "  ├─ %-6s ", writer->msg);
        }
        return;
    } else {
        fprintf(writer->output, "  │ ");
    }

    for (int i = 0; i < writer->indentation; ++i) {
        if (i < writer->indentation-2)
            fprintf(writer->output, writer->last[i] ? "          " : "        │ ");
        else if (i == writer->indentation-2)
            fprintf(writer->output, (is_last) ? "        └─" : "        ├─");
        else
            fprintf(writer->output, " %-6s ", writer->msg);
    }
}

#define generate_name_(name, line) name ## line
#define generate_name(name, line) generate_name_(name, line)
#define print_indentation(is_last, ...) writer->last[writer->indentation > 1 ? writer->indentation-2 : 0] = is_last; char generate_name(buffer, __LINE__) [12] = { 0 }; snprintf(generate_name(buffer, __LINE__), 12, __VA_ARGS__); writer->msg = generate_name(buffer, __LINE__); print_indentation_(writer, is_last)
#define printer_rst() writer->msg = ""


static inline Location location_of_node(UntypedAst* ast, Node* node) {
    return location_of(ast->tokens.source.data, ast->tokens.indices[node->base.start]);
}

static inline NodeId id_of(UntypedAst* ast, Node* node) {
    return (NodeId) (node - ast->nodes);
}


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static void tree_write_expression(TreeWriter* writer, const Node* node);
static void tree_write_statement(TreeWriter* writer, const Node* node);


static void tree_write_literal(TreeWriter* writer, const NodeLiteral* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* repr = lexer_repr_of(writer->ast->tokens, node->base.start);
    const char* type_name = literal_type_name(node->type);
    const char* path = writer->ast->tokens.name.data;
    fprintf(writer->output, "Literal: id=%d, repr='%s', type='%s' @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), repr, type_name, path, location.row, location.column);
}

static void tree_write_identifier(TreeWriter* writer, const NodeIdentifier* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* repr = lexer_repr_of(writer->ast->tokens, node->base.start);
    const char* path = writer->ast->tokens.name.data;
    fprintf(writer->output, "Identifier: id=%d, repr='%s' @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), repr, path, location.row, location.column);

}

static void tree_write_binary(TreeWriter* writer, const NodeBinary* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* repr = lexer_repr_of(writer->ast->tokens, node->base.start);
    const char* path = writer->ast->tokens.name.data;

    fprintf(writer->output, "Binary: id=%d, repr='%s', op='%s' @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), repr, binary_op_repr(*node), path, location.row, location.column);

    writer->indentation += 1;
    print_indentation(0, "left");
    tree_write_expression(writer, node->left);
    print_indentation(1, "right");
    tree_write_expression(writer, node->right);
    writer->indentation -= 1;

    printer_rst();
}

static void tree_write_call(TreeWriter* writer, const NodeCall call) {
    (void)writer;
    (void)call;
}

static void tree_write_var_decl(TreeWriter* writer, const NodeVarDecl* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* path = writer->ast->tokens.name.data;

    fprintf(writer->output, "VarDecl: id=%d, name='%s' @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), node->name, path, location.row, location.column);

    writer->indentation += 1;
    print_indentation(1, "expr");
    tree_write_expression(writer, node->expression);
    writer->indentation -= 1;

    printer_rst();
}

static void tree_write_assignment(TreeWriter* writer, const NodeAssign* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* path = writer->ast->tokens.name.data;

    fprintf(writer->output, "Assign: id=%d, name='%s' @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), node->name, path, location.row, location.column);

    writer->indentation += 1;
    print_indentation(1, "expr");
    tree_write_expression(writer, node->expression);
    writer->indentation -= 1;

    printer_rst();
}

static void tree_write_block(TreeWriter* writer, const NodeBlock* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* path = writer->ast->tokens.name.data;

    fprintf(writer->output, "Block: id=%d @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), path, location.row, location.column);

    writer->indentation += 1;
    Node* stmt = NULL;
    int i = 0;
    while ((stmt = *(node->nodes + i)) != NULL) {
        print_indentation(node->nodes[i+1] == NULL, "stmt%d", i);
        tree_write_statement(writer, stmt);
        i += 1;
    }
    writer->indentation -= 1;
    printer_rst();
}

static void tree_write_fun_decl(TreeWriter* writer, const NodeFunDecl fun_decl) {
    (void)writer;
    (void)fun_decl;
}

static void tree_write_if_stmt(TreeWriter* writer, const NodeIf* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* path = writer->ast->tokens.name.data;

    fprintf(writer->output, "If: id=%d @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), path, location.row, location.column);

    writer->indentation += 1;
    print_indentation(0, "cond");
    tree_write_expression(writer, node->condition);
    print_indentation(node->else_block == NULL, "then");
    tree_write_block(writer, node->then_block);
    if (node->else_block != NULL) {
        print_indentation(1, "else");
        tree_write_block(writer, node->else_block);
    }
    writer->indentation -= 1;
    printer_rst();
}

static void tree_write_while_stmt(TreeWriter* writer, const NodeWhile* node) {
    Location location = location_of_node(writer->ast, (Node*)node);
    const char* path = writer->ast->tokens.name.data;

    fprintf(writer->output, "While: id=%d @ %s:%d:%d\n", id_of(writer->ast, (Node*)node), path, location.row, location.column);

    writer->indentation += 1;
    print_indentation(0, "cond");
    tree_write_expression(writer, node->condition);
    print_indentation(node->else_block == NULL, "then");
    tree_write_block(writer, node->then_block);
    if (node->else_block != NULL) {
        print_indentation(1, "else");
        tree_write_block(writer, node->else_block);
    }
    writer->indentation -= 1;
    printer_rst();
}

static void tree_write_expression(TreeWriter* writer, const Node* node) {
    assert(node_is_expression(node) && "Not an expression");
    switch (node->kind) {
        case NodeKind_Literal:
            tree_write_literal(writer, &node->literal);
            break;
        case NodeKind_Identifier:
            tree_write_identifier(writer, &node->identifier);
            break;
        case NodeKind_Binary:
            tree_write_binary(writer, &node->binary);
            break;
        case NodeKind_Call:
            tree_write_call(writer, node->call);
            break;
        default:
            assert(0 && "not implemented");
    }
}

static void tree_write_statement(TreeWriter* writer, const Node* node) {
    assert(node_is_statement(node) && "Not a statement");
    switch (node->kind) {
        case NodeKind_Literal:
        case NodeKind_Identifier:
        case NodeKind_Binary:
        case NodeKind_Call:
            tree_write_expression(writer, node);
            break;
        case NodeKind_Assign:
            tree_write_assignment(writer, &node->assign);
            break;
        case NodeKind_VarDecl:
            tree_write_var_decl(writer, &node->var_decl);
            break;
        case NodeKind_If:
            tree_write_if_stmt(writer, &node->if_stmt);
            break;
        case NodeKind_While:
            tree_write_while_stmt(writer, &node->while_stmt);
            break;
        case NodeKind_Block:
            tree_write_block(writer, &node->block);
            break;
        case NodeKind_FunDecl:
            tree_write_fun_decl(writer, node->fun_decl);
            break;
        default:
            assert(0 && "not implemented");
    }
}

void tree_write(UntypedAst ast, FILE* output) {
    TreeWriter writer = {
        .ast = &ast,
        .output = output,
        .indentation = 0,
    };
    Node* node = ast.start;
    tree_write_statement(&writer, node);
}

