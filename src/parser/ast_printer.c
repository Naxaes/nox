#include "ast_printer.h"

#include "location.h"

typedef struct {
    Visitor visitor;
    GrammarTree* ast;
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


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static void visit_literal(AstPrinter* printer, const NodeLiteral* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* type_name = literal_type_name(node->type);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Literal: id=%d, repr='%s', type='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, type_name, path, location.row, location.column);
}

static void visit_identifier(AstPrinter* printer, const NodeIdentifier* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Identifier: id=%d, repr='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, path, location.row, location.column);
}


static void visit_unary(AstPrinter* printer, const NodeUnary* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Unary: id=%d, repr='%s', op='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, unary_op_repr(node->op), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    visit((Visitor*) printer, node->expr);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_binary(AstPrinter* printer, const NodeBinary* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Binary: id=%d, repr='%s', op='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, binary_op_repr(node->op), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "left");
    visit((Visitor*) printer, node->left);
    print_indentation(1, "right");
    visit((Visitor*) printer, node->right);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_call(AstPrinter* printer, const NodeCall* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunCall: id=%d, name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), node->name, path, location.row, location.column);

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

static void visit_access(AstPrinter* printer, const NodeAccess* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Access: id=%d @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "left");
    visit((Visitor*) printer, node->left);
    print_indentation(1, "right");
    visit((Visitor*) printer, node->right);
    printer->indentation -= 1;
    printer_rst();
}

static void visit_type(AstPrinter* printer, const NodeType* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    const char* repr = (node->name < (const char*)10) ? literal_type_repr((LiteralType)(size_t)node->name) : lexer_repr_of(printer->ast->tokens, node->base.start);
    fprintf(printer->output, "Type: id=%d, name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, path, location.row, location.column);
}

static void visit_var_decl(AstPrinter* printer, const NodeVarDecl* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "VarDecl: id=%d, name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), node->name, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    visit((Visitor*) printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_assign(AstPrinter* printer, const NodeAssign* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Assign: id=%d, name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), node->name, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    visit((Visitor*) printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_block(AstPrinter* printer, const NodeBlock* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Block: id=%d, parent='%d', block_id='%d' @ %s:%d:%d\n",
            node_id(printer->ast, (Node *) node), node->parent, node->id, path, location.row, location.column);

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
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunParam: id=%d, name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, path, location.row, location.column);
}

static void visit_fun_body(AstPrinter* printer, const NodeFunBody* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunBody: id=%d, parent='%d', block_id='%d' decls='%d' @ %s:%d:%d\n",
            node_id(printer->ast, (Node *) node), node->parent, node->id, node->decls, path, location.row, location.column);

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

static void visit_fun_decl(AstPrinter* printer, const NodeFunDecl* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunDecl: id=%d, repr='%s', name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, node->name, path, location.row, location.column);

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
    visit_fun_body(printer, node->body);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_return_stmt(AstPrinter* printer, const NodeReturn* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Return: id=%d, repr='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    visit((Visitor*) printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void visit_if_stmt(AstPrinter* printer, const NodeIf* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "If: id=%d @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "cond");
    visit((Visitor*) printer, node->condition);
    print_indentation(node->else_block == NULL, "then");
    visit(printer, (Node*) node->then_block);
    if (node->else_block != NULL) {
        print_indentation(1, "else");
        visit(printer, (Node*) node->else_block);
    }
    printer->indentation -= 1;
    printer_rst();
}

static void visit_while_stmt(AstPrinter* printer, const NodeWhile* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "While: id=%d @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), path, location.row, location.column);

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

static void visit_init_arg(AstPrinter* printer, const NodeInitArg* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "InitArg: id=%d, name='%s', offset=%d @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), node->name, node->offset, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    visit((Visitor*) printer, node->expr);
    printer->indentation -= 1;
}

static void visit_init(AstPrinter* printer, const NodeInit* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Init: id=%d, name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), node->name, path, location.row, location.column);

    printer->indentation += 1;
    NodeInitArg* stmt = NULL;
    int i = 0;
    if (node->args != NULL) {
        while ((stmt = *(node->args + i)) != NULL) {
            print_indentation(node->args[i+1] == NULL, "arg%d", i);
            visit((Visitor*) printer, (Node*) stmt);
            i += 1;
        }
    }
    printer->indentation -= 1;
    printer_rst();
}

static void visit_struct_field(AstPrinter* printer, const NodeStructField* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "StructField: id=%d, repr='%s', name='%s' @ %s:%d:%d\n",
            node_id(printer->ast, (Node *) node), repr, node->name, path, location.row, location.column);
    printer->indentation += 1;
    print_indentation(0, "type");
    visit(printer, (Node*) node->type);
    if (node->expr != NULL) {
        print_indentation(1, "expr");
        visit((Visitor*) printer, node->expr);
    }
    printer->indentation -= 1;
}

static void visit_struct_decl(AstPrinter* printer, const NodeStruct* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Struct: id=%d, repr='%s', name='%s' @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), repr, node->name, path, location.row, location.column);

    printer->indentation += 1;
    Node* stmt = NULL;
    int i = 0;
    if (node->nodes != NULL) {
        while ((stmt = *(node->nodes + i)) != NULL) {
            print_indentation(node->nodes[i+1] == NULL, "field%d", i);
            visit((Visitor*) printer, stmt);
            i += 1;
        }
    }
    printer->indentation -= 1;
    printer_rst();
}

static void visit_module(AstPrinter* printer, const NodeModule* node) {
    Location location = node_location(printer->ast, (Node *) node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Module: id=%d @ %s:%d:%d\n", node_id(printer->ast, (Node *) node), path, location.row, location.column);

    printer->indentation += 1;
    Node* stmt = NULL;

    int i = 0;
    if (node->decls != NULL) {
        while ((stmt = *(node->decls + i)) != NULL) {
            print_indentation(node->decls[i+1] == NULL, "decl%d", i);
            visit((Visitor*) printer, stmt);
            i += 1;
        }
    }

    i = 0;
    if (node->stmts != NULL) {
        while ((stmt = *(node->stmts + i)) != NULL) {
            print_indentation(node->stmts[i+1] == NULL, "stmt%d", i);
            visit((Visitor*) printer, stmt);
            i += 1;
        }
    }

    printer->indentation -= 1;
    printer_rst();
}



void ast_print(GrammarTree ast, FILE* output) {
    AstPrinter printer = {
        .visitor = {
#define X(upper, lower, flags, body) .visit_##lower = (Visit##upper##Fn) visit_##lower,
            ALL_NODES(X)
#undef X
        },
        .ast = &ast,
        .output = output,
        .indentation = 0,
    };

    Node* node = ast.start;
    visit((Visitor*) &printer, node);
}
