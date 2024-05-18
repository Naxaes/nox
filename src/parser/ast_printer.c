#include "ast_printer.h"

#include "location.h"

STATIC_ASSERT(NODE_KIND_COUNT == 14, Node_count_has_changed);
typedef struct AstPrinter {
    void (*visit_literal)(struct AstPrinter* self, const NodeLiteral* node);
    void (*visit_identifier)(struct AstPrinter* self, const NodeIdentifier* node);
    void (*visit_binary)(struct AstPrinter* self, const NodeBinary* node);
    void (*visit_call)(struct AstPrinter* self, const NodeCall* node);
    void (*visit_type)(struct AstPrinter* self, const NodeType* node);
    void (*visit_assign)(struct AstPrinter* self, const NodeAssign* node);
    void (*visit_var_decl)(struct AstPrinter* self, const NodeVarDecl* node);
    void (*visit_block)(struct AstPrinter* self, const NodeBlock* node);
    void (*visit_fun_body)(struct AstPrinter* self, const NodeFunBody* node);
    void (*visit_fun_param)(struct AstPrinter* self, const NodeFunParam* node);
    void (*visit_fun_decl)(struct AstPrinter* self, const NodeFunDecl* node);
    void (*visit_return_stmt)(struct AstPrinter* self, const NodeReturn* node);
    void (*visit_if_stmt)(struct AstPrinter* self, const NodeIf* node);
    void (*visit_while_stmt)(struct AstPrinter* self, const NodeWhile* node);
    void (*visit_module)(struct AstPrinter* self, const NodeModule* node);
    
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
    }


    for (int i = 0; i < printer->indentation; ++i) {
        if (i == 0 && !printer->last[0]) {
            if (i == printer->indentation-2) {
                fprintf(printer->output, (is_last) ? "  │     └─" : "  │     ├─");
            } else {
                fprintf(printer->output, (printer->last[i+1]) ? "  │       " : "  │     │ ");
            }
        } else if (i == printer->indentation-2) {
            fprintf(printer->output, (is_last) ? "        └─" : "        ├─");
        } else if (i == printer->indentation-1) {
            fprintf(printer->output, " %-6s ", printer->msg);
        } else if (printer->last[i+1]) {
            fprintf(printer->output, "          ");
        } else {
            fprintf(printer->output, "        │ ");
        }
        fflush(printer->output);
    }
}

#define generate_name_(name, line) name ## line
#define generate_name(name, line) generate_name_(name, line)
#define print_indentation(is_last, ...) printer->last[printer->indentation-1] = is_last; char generate_name(buffer, __LINE__) [12] = { 0 }; snprintf(generate_name(buffer, __LINE__), 12, __VA_ARGS__); printer->msg = generate_name(buffer, __LINE__); print_indentation_(printer, is_last)
#define printer_rst() printer->msg = ""


static inline Location location_of_node(GrammarTree* ast, Node* node) {
    return location_of(ast->tokens.source.data, ast->tokens.source_offsets[node->base.start]);
}

static inline NodeId id_of(GrammarTree* ast, Node* node) {
    return (NodeId) (node - ast->nodes);
}


/* ---------------------------- CHECKER VISITOR -------------------------------- */
static void ast_printer_visit(AstPrinter* printer, const Node* node) {
    STATIC_ASSERT(NODE_KIND_COUNT == 14, Node_count_has_changed);
    switch (node->kind) {
        case NodeKind_Literal:
            printer->visit_literal(printer, (NodeLiteral *) node);
            break;
        case NodeKind_Identifier:
            printer->visit_identifier(printer, (NodeIdentifier *) node);
            break;
        case NodeKind_Binary:
            printer->visit_binary(printer, (NodeBinary *) node);
            break;
        case NodeKind_Call:
            printer->visit_call(printer, (NodeCall *) node);
            break;
        case NodeKind_Type:
            printer->visit_type(printer, (NodeType *) node);
            break;
        case NodeKind_Assign:
            printer->visit_assign(printer, (NodeAssign *) node);
            break;
        case NodeKind_VarDecl:
            printer->visit_var_decl(printer, (NodeVarDecl *) node);
            break;
        case NodeKind_Block:
            printer->visit_block(printer, (NodeBlock *) node);
            break;
        case NodeKind_FunParam:
            printer->visit_fun_param(printer, (NodeFunParam *) node);
            break;
        case NodeKind_FunBody:
            printer->visit_fun_body(printer, (NodeFunBody *) node);
            break;
        case NodeKind_FunDecl:
            printer->visit_fun_decl(printer, (NodeFunDecl *) node);
            break;
        case NodeKind_Return:
            printer->visit_return_stmt(printer, (NodeReturn *) node);
            break;
        case NodeKind_If:
            printer->visit_if_stmt(printer, (NodeIf *) node);
            break;
        case NodeKind_While:
            printer->visit_while_stmt(printer, (NodeWhile *) node);
            break;
        case NodeKind_Module:
            printer->visit_module(printer, (NodeModule *) node);
            break;
    }
}

static void ast_printer_visit_literal(AstPrinter* printer, const NodeLiteral* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* type_name = literal_type_name(node->type);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Literal: id=%d, repr='%s', type='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, type_name, path, location.row, location.column);
}

static void ast_printer_visit_identifier(AstPrinter* printer, const NodeIdentifier* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;
    fprintf(printer->output, "Identifier: id=%d, repr='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, path, location.row, location.column);

}

static void ast_printer_visit_binary(AstPrinter* printer, const NodeBinary* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Binary: id=%d, repr='%s', op='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, binary_op_repr(node->op), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "left");
    ast_printer_visit(printer, node->left);
    print_indentation(1, "right");
    ast_printer_visit(printer, node->right);
    printer->indentation -= 1;

    printer_rst();
}

static void ast_printer_visit_call(AstPrinter* printer, const NodeCall* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunCall: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->name, path, location.row, location.column);

    if (node->args != NULL) {
        printer->indentation += 1;
        Node* stmt = NULL;
        int i = 0;
        while ((stmt = *(node->args + i)) != NULL) {
            print_indentation(node->args[i+1] == NULL, "arg%d", i);
            ast_printer_visit(printer, stmt);
            i += 1;
        }
        printer->indentation -= 1;
    }
    printer_rst();
}

static void ast_printer_visit_type(AstPrinter* printer, const NodeType* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    const char* repr = (node->name < (const char*)10) ? literal_type_repr((LiteralType)(size_t)node->name) : lexer_repr_of(printer->ast->tokens, node->base.start);
    fprintf(printer->output, "Type: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, path, location.row, location.column);
}

static void ast_printer_visit_var_decl(AstPrinter* printer, const NodeVarDecl* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "VarDecl: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->name, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    ast_printer_visit(printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void ast_printer_visit_assign(AstPrinter* printer, const NodeAssign* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Assign: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->name, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    ast_printer_visit(printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void ast_printer_visit_block(AstPrinter* printer, const NodeBlock* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Block: id=%d, parent='%d', block_id='%d' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->parent, node->id, path, location.row, location.column);

    printer->indentation += 1;
    Node* stmt = NULL;
    int i = 0;
    if (node->nodes != NULL) {
        while ((stmt = *(node->nodes + i)) != NULL) {
            print_indentation(node->nodes[i+1] == NULL, "stmt%d", i);
            ast_printer_visit(printer, stmt);
            i += 1;
        }
    }
    printer->indentation -= 1;
    printer_rst();
}

static void ast_printer_visit_fun_param(AstPrinter* printer, const NodeFunParam* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunParam: id=%d, name='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, path, location.row, location.column);
}

static void ast_printer_visit_fun_body(AstPrinter* printer, const NodeFunBody* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "FunBody: id=%d, parent='%d', block_id='%d' decls='%d' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), node->parent, node->id, node->decls, path, location.row, location.column);

    printer->indentation += 1;
    Node* stmt = NULL;
    int i = 0;
    if (node->nodes != NULL) {
        while ((stmt = *(node->nodes + i)) != NULL) {
            print_indentation(node->nodes[i+1] == NULL, "stmt%d", i);
            ast_printer_visit(printer, stmt);
            i += 1;
        }
    }
    printer->indentation -= 1;
    printer_rst();
}

static void ast_printer_visit_fun_decl(AstPrinter* printer, const NodeFunDecl* node) {
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
            ast_printer_visit(printer, (Node*)param);
            i += 1;
        }
    }

    print_indentation(node->return_type == NULL, "body");
    ast_printer_visit_fun_body(printer, node->body);

    if (node->return_type) {
        print_indentation(1, "return");
        ast_printer_visit(printer, node->return_type);
    }

    printer->indentation -= 1;


    printer_rst();
}

static void ast_printer_visit_return_stmt(AstPrinter* printer, const NodeReturn* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* repr = lexer_repr_of(printer->ast->tokens, node->base.start);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Return: id=%d, repr='%s' @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), repr, path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(1, "expr");
    ast_printer_visit(printer, node->expression);
    printer->indentation -= 1;

    printer_rst();
}

static void ast_printer_visit_if_stmt(AstPrinter* printer, const NodeIf* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "If: id=%d @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "cond");
    ast_printer_visit(printer, node->condition);
    print_indentation(node->else_block == NULL, "then");
    ast_printer_visit_block(printer, node->then_block);
    if (node->else_block != NULL) {
        print_indentation(1, "else");
        ast_printer_visit_block(printer, node->else_block);
    }
    printer->indentation -= 1;
    printer_rst();
}

static void ast_printer_visit_while_stmt(AstPrinter* printer, const NodeWhile* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "While: id=%d @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), path, location.row, location.column);

    printer->indentation += 1;
    print_indentation(0, "cond");
    ast_printer_visit(printer, node->condition);
    print_indentation(node->else_block == NULL, "then");
    ast_printer_visit_block(printer, node->then_block);
    if (node->else_block != NULL) {
        print_indentation(1, "else");
        ast_printer_visit_block(printer, node->else_block);
    }
    printer->indentation -= 1;
    printer_rst();
}


static void ast_printer_visit_module(AstPrinter* printer, const NodeModule* node) {
    Location location = location_of_node(printer->ast, (Node*)node);
    const char* path = printer->ast->tokens.name.data;

    fprintf(printer->output, "Module: id=%d @ %s:%d:%d\n", id_of(printer->ast, (Node*)node), path, location.row, location.column);

    printer->indentation += 1;

    for (int i = 0; i < node->decl_count; ++i) {
        print_indentation(i == node->decl_count-1 && node->stmt_count == 0, "decl%d", i);
        ast_printer_visit(printer, node->decls[i]);
    }

    for (int i = 0; i < node->stmt_count; ++i) {
        print_indentation(i == node->stmt_count-1, "stmt%d", i);
        ast_printer_visit(printer, node->stmts[i]);
    }

    printer->indentation -= 1;
    printer_rst();
}



void ast_print(GrammarTree ast, FILE* output) {
    AstPrinter printer = {
        .visit_literal=ast_printer_visit_literal, 
        .visit_identifier=ast_printer_visit_identifier, 
        .visit_binary=ast_printer_visit_binary, 
        .visit_call=ast_printer_visit_call, 
        .visit_type=ast_printer_visit_type, 
        .visit_assign=ast_printer_visit_assign, 
        .visit_var_decl=ast_printer_visit_var_decl, 
        .visit_block=ast_printer_visit_block, 
        .visit_fun_body=ast_printer_visit_fun_body, 
        .visit_fun_param=ast_printer_visit_fun_param, 
        .visit_fun_decl=ast_printer_visit_fun_decl, 
        .visit_return_stmt=ast_printer_visit_return_stmt, 
        .visit_if_stmt=ast_printer_visit_if_stmt, 
        .visit_while_stmt=ast_printer_visit_while_stmt, 
        .visit_module=ast_printer_visit_module,
        
        .ast = &ast,
        .output = output,
        .indentation = 0,
    };

    Node* node = ast.start;
    ast_printer_visit(&printer, node);
}
