#include "ast_printer.h"

#include "location.h"

typedef struct {
    Visitor visitor;
    UntypedAst* ast;
    FILE* output;
    const char* msg;
    int   indentation;
    int   last[24];
} AstPrinter;

static void print_indentation_(AstPrinter* printer, const int is_last) {
    if (printer->indentation == 0)
        return;


    if (printer->indentation == 1) {
        if (is_last) {
            fprintf(printer->output, "  └─ %-6s ", printer->msg);
        } else {
            fprintf(printer->output, "  ├─ %-6s ", printer->msg);
        }
        return;
    } else {
        fprintf(printer->output, "  │ ");
    }

    for (int i = 0; i < printer->indentation; ++i) {
        if (i < printer->indentation-2)
            fprintf(printer->output, printer->last[i] ? "          " : "        │ ");
        else if (i == printer->indentation-2)
            fprintf(printer->output, (is_last) ? "        └─" : "        ├─");
        else
            fprintf(printer->output, " %-6s ", printer->msg);
    }
}

#define generate_name_(name, line) name ## line
#define generate_name(name, line) generate_name_(name, line)
#define print_indentation(is_last, ...) printer->last[printer->indentation > 1 ? printer->indentation-2 : 0] = is_last; char generate_name(buffer, __LINE__) [12] = { 0 }; snprintf(generate_name(buffer, __LINE__), 12, __VA_ARGS__); printer->msg = generate_name(buffer, __LINE__); print_indentation_(printer, is_last)
#define printer_rst() printer->msg = ""


static inline Location location_of_node(UntypedAst* ast, Node* node) {
    return location_of(ast->tokens.source.data, ast->tokens.source_offsets[node->base.start]);
}

static inline NodeId id_of(UntypedAst* ast, Node* node) {
    return (NodeId) (node - ast->nodes);
}


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static void visit_literal(AstPrinter* printer, const NodeLiteral* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* type_name = literal_type_name(node->type);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Literal: id=%d, repr='%s', type='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, type_name, path, location.row, location.column);
}

static void visit_identifier(AstPrinter* printer, const NodeIdentifier* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Identifier: id=%d, repr='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, path, location.row, location.column);

}

static void visit_binary(AstPrinter* printer, const NodeBinary* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Binary: id=%d, repr='%s', op='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, binary_op_repr(node->op), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "left");
    visit((Visitor*) printer, node->left);
    print_indentation(1, "right");
    visit((Visitor*) printer, node->right);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_call(AstPrinter* printer, const NodeCall* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunCall: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->name, path, location.row, location.column);

    if (node->args != NULL) {
        printer->indentation += 1;
        Node* stmt = NULL;
        int i = 0;
        while ((stmt = *(node->args + i)) != NULL) {
            print_indentation(node->args[i+1] == NULL, "arg%d", i);
            visit((Visitor*) printer, stmt);
            i += 1;
        }
        printer->indentation -= 1;
    }
    printer_rst();
}

static void visit_var_decl(AstPrinter* printer, const NodeVarDecl* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "VarDecl: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->name, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    visit((Visitor*) printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_assign(AstPrinter* printer, const NodeAssign* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Assign: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->name, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    visit((Visitor*) printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_block(AstPrinter* printer, const NodeBlock* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Block: id=%d, parent='%d', block_id='%d' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->parent, node->id, path, location.row, location.column);

    printer->indentation += 1;
    Node* stmt = NULL;
    int i = 0;
    if (node->nodes != NULL) {
        while ((stmt = *(node->nodes + i)) != NULL) {
            print_indentation(node->nodes[i+1] == NULL, "stmt%d", i);
            visit((Visitor*) printer, stmt);
            i += 1;
        }
    }
    printer->indentation -= 1;
    printer_rst();
}

static void visit_fun_param(AstPrinter* printer, const NodeFunParam* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunParam: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, path, location.row, location.column);
}

static void visit_fun_decl(AstPrinter* printer, const NodeFunDecl* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunDecl: id=%d, repr='%s', name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, node->name, path, location.row, location.column);

    printer->indentation += 1;

    if (node->params != NULL) {
        NodeFunParam* param = NULL;
        int i = 0;
        while ((param = *(node->params + i)) != NULL) {
            print_indentation(0, "param%d", i);
            visit((Visitor*) printer, (Node*)param);
            i += 1;
        }
    }

    print_indentation(1, "body");
    visit_block(printer, node->block);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_if_stmt(AstPrinter* printer, const NodeIf* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "If: id=%d @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "cond");
    visit((Visitor*) printer, node->condition);
    print_indentation(node->else_block == NULL, "then");
    visit_block(printer, node->then_block);
    if (node->else_block != NULL) {
        print_indentation(1, "else");
        visit_block(printer, node->else_block);
    }
    printer->indentation -= 1;
    printer_rst();
}

static void visit_while_stmt(AstPrinter* printer, const NodeWhile* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "While: id=%d @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "cond");
    visit((Visitor*) printer, node->condition);
    print_indentation(node->else_block == NULL, "then");
    visit_block(printer, node->then_block);
    if (node->else_block != NULL) {
        print_indentation(1, "else");
        visit_block(printer, node->else_block);
    }
    printer->indentation -= 1;
    printer_rst();
}


void ast_print(UntypedAst ast, FILE* output) {
    AstPrinter printer = {
        .visitor = {
            .visit_literal = (void*) visit_literal,
            .visit_identifier = (void*) visit_identifier,
            .visit_binary = (void*) visit_binary,
            .visit_call = (void*) visit_call,
            .visit_assign = (void*) visit_assign,
            .visit_var_decl = (void*) visit_var_decl,
            .visit_block = (void*) visit_block,
            .visit_fun_param = (void*) visit_fun_param,
            .visit_fun_decl = (void*) visit_fun_decl,
            .visit_if_stmt = (void*) visit_if_stmt,
            .visit_while_stmt = (void*) visit_while_stmt,
        },
        .ast = &ast,
        .output = output,
        .indentation = 0,
    };

    Node* node = ast.start;
    visit((Visitor*) &printer, node);
}
