#include "parser.h"
#include "error.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>





/* ---------------------------- PARSER IMPL -------------------------------- */
typedef struct {
    const TokenArray tokens;
    TokenIndex token_index;

    NodeBlock*  current_block;
    int         current_decl_count;

    NodeId     block_count;
    Node**     stack;
    NodeId     stack_count;

    Node*      nodes;
    NodeId     node_count;
    Node**     views;
    NodeId     view_count;
} Parser;

void parser_free(Parser* parser) {
    free(parser->nodes);
    free(parser->views);
    free(parser->stack);
    token_array_free(parser->tokens);
}

GrammarTree parser_to_ast(Parser* parser, Node* start) {
    free(parser->stack);
    return (GrammarTree) {
        parser->tokens,
        parser->nodes,
        start,
        parser->views,
        parser->block_count
    };
}

/* ---------------------------- PARSER HELPERS -------------------------------- */
static inline Token current(const Parser* parser) {
    return parser->tokens.tokens[parser->token_index];
}

static inline Token peek(const Parser* parser) {
    if (parser->token_index + 1 >= parser->tokens.size)
        return Token_Eof;
    return parser->tokens.tokens[parser->token_index + 1];
}

static inline TokenIndex advance(Parser* parser) {
    return parser->token_index++;
}

static inline const char* repr_of_current(const Parser* parser) {
    return lexer_repr_of(parser->tokens, parser->token_index);
}

static inline Node* add_node(Parser* parser, Node node) {
    NodeId id = parser->node_count++;
    parser->nodes[id] = node;
    return &parser->nodes[id];
}

static inline NodeId reserve_node(Parser* parser) {
    NodeId id = parser->node_count++;
    return id;
}

static inline Node* set_node(Parser* parser, NodeId id, Node node) {
    parser->nodes[id] = node;
    return &parser->nodes[id];
}

// Sorts the nodes in the stack by declaration order.
// This is so we can visit function declarations before they are used.
static int sort_nodes_by_fun_decl(Node** nodes, int count) {
    int fun_count = 0;
    for (int i = 0; i < count; ++i) {
        if (nodes[i]->kind == NodeKind_FunDecl) {
            for (int j = i; j > fun_count; --j) {
                if (nodes[j - 1]->kind == NodeKind_FunDecl) {
                    Node* temp = nodes[j];
                    nodes[j] = nodes[j - 1];
                    nodes[j - 1] = temp;
                } else {
                    break;
                }
            }
            fun_count += 1;
        }
    }
    return fun_count;
}

static inline NodeId stack_snapshot(Parser* parser) {
    return parser->stack_count;
}

static inline void stack_push(Parser* parser, Node* node) {
    parser->stack[parser->stack_count++] = node;
}

static inline Node** stack_restore(Parser* parser, NodeId snapshot) {
    if (snapshot == parser->stack_count) {
        return NULL;
    }

    Node** nodes = parser->views + parser->view_count;
    for (size_t i = snapshot; i < parser->stack_count; ++i) {
        parser->views[parser->view_count++] = parser->stack[i];
    }
    parser->views[parser->view_count++] = NULL;
    parser->stack_count = snapshot;
    return nodes;
}


/* ---------------------------- PARSER VISITOR -------------------------------- */
static Node* number(Parser*);
static Node* real(Parser*);
static Node* string(Parser* parser);
static Node* identifier(Parser*);
static Node* group(Parser*);

static Node* binary(Parser* parser, Node* left);
static Node* call(Parser* parser, Node* left);

static Node* statement(Parser* parser);
static Node* expression(Parser* parser);


typedef enum {
    Precedence_None,
    Precedence_Assignment,  // =
    Precedence_Or,          // or
    Precedence_And,         // and
    Precedence_Equality,    // == !=
    Precedence_Comparison,  // < > <= >=
    Precedence_Term,        // + -
    Precedence_Factor,      // * /
    Precedence_Unary,       // ! -
    Precedence_Call,        // . ()
    Precedence_Primary
} Precedence;

typedef Node* (*ParsePrefixFn)(Parser*);
typedef Node* (*ParseInfixFn)(Parser*, Node*);
typedef struct {
    ParsePrefixFn prefix;
    ParseInfixFn  infix;
    Precedence precedence;
} ParseRule;

ParseRule rules[30] = {
        [Token_Number]              = { number,       NULL,         Precedence_None},
        [Token_Real]                = { real,         NULL,         Precedence_None},
        [Token_String]              = { string,       NULL,         Precedence_None},
        [Token_Identifier]          = { identifier,   NULL,         Precedence_None},
        [Token_Minus]               = { NULL,         binary,       Precedence_Term},
        [Token_Plus]                = { NULL,         binary,       Precedence_Term},
        [Token_Asterisk]            = { NULL,         binary,       Precedence_Factor},
        [Token_Slash]               = { NULL,         binary,       Precedence_Factor},
        [Token_Percent]             = { NULL,         binary,       Precedence_Factor},
        [Token_Less]                = { NULL,         binary,       Precedence_Comparison},
        [Token_Less_Equal]          = { NULL,         binary,       Precedence_Comparison},
        [Token_Equal_Equal]         = { NULL,         binary,       Precedence_Equality},
        [Token_Bang_Equal]          = { NULL,         binary,       Precedence_Equality},
        [Token_Greater_Equal]       = { NULL,         binary,       Precedence_Comparison},
        [Token_Greater]             = { NULL,         binary,       Precedence_Comparison},
        [Token_Bang]                = { NULL,         NULL,         Precedence_None},
        [Token_Equal]               = { NULL,         NULL,         Precedence_None},
        [Token_Colon]               = { NULL,         NULL,         Precedence_None},
        [Token_Colon_Equal]         = { NULL,         NULL,         Precedence_None},
        [Token_If]                  = { NULL,         NULL,         Precedence_None},
        [Token_Else]                = { NULL,         NULL,         Precedence_None},
        [Token_While]               = { NULL,         NULL,         Precedence_None},
        [Token_Fun]                 = { NULL,         NULL,         Precedence_None},
        [Token_Return]              = { NULL,         NULL,         Precedence_None},
        [Token_Open_Paren]          = { group,        call,         Precedence_Call},
        [Token_Close_Paren]         = { NULL,         NULL,         Precedence_None},
        [Token_Open_Brace]          = { NULL,         NULL,         Precedence_None},
        [Token_Close_Brace]         = { NULL,         NULL,         Precedence_None},
        [Token_Comma]               = { NULL,         NULL,         Precedence_None},
        [Token_Eof]                 = { NULL,         NULL,         Precedence_None},
};


static Node* precedence(Parser* parser, Precedence precedence) {
    Token token = current(parser);

    ParsePrefixFn prefix = rules[token].prefix;
    if (prefix == NULL) {
        fprintf(stderr, "Expect expression\n");
        return 0;
    }

    Node* left = prefix(parser);

    while (1) {
        token = current(parser);
        Precedence next_precedence = rules[token].precedence;
        if (precedence > next_precedence) {
            break;
        }

        ParseInfixFn infix = rules[token].infix;
        left = infix(parser, left);
    }

    return left;
}


static Node* number(Parser* parser) {
    assert(current(parser) == Token_Number && "Expected number token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    i64 value = strtoll(repr, NULL, 10);
    advance(parser);

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = LiteralType_Integer, .value.integer = value };
    return add_node(parser, (Node) { .literal = literal });
}


static Node* real(Parser* parser) {
    assert(current(parser) == Token_Real && "Expected real token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    f64 value = strtod(repr, NULL);
    advance(parser);

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = LiteralType_Real, .value.real = value };
    return add_node(parser, (Node) { .literal = literal });
}


static Node* string(Parser* parser) {
    assert(current(parser) == Token_String && "Expected string token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = LiteralType_String, .value.string = repr };
    return add_node(parser, (Node) { .literal = literal });
}


static Node* identifier(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    NodeIdentifier ident = {
        node_base_identifier(start, start),
        .name = repr,
    };
    return add_node(parser, node_identifier(ident));
}


static Node* binary(Parser* parser, Node* left) {
    Token token = current(parser);
    assert((
        token_group(token) == TokenGroup_Binary_Arithmetic_Operator ||
        token_group(token) == TokenGroup_Binary_Comparison_Operator
    ) && "Expected binary operator");
    TokenIndex start = parser->token_index;

    static const BinaryOp bin_op_map[] = {
        [Token_Plus]        = BinaryOp_Add,
        [Token_Minus]       = BinaryOp_Sub,
        [Token_Asterisk]    = BinaryOp_Mul,
        [Token_Slash]       = BinaryOp_Div,
        [Token_Percent]     = BinaryOp_Mod,
        [Token_Less]        = BinaryOp_Lt,
        [Token_Less_Equal]  = BinaryOp_Le,
        [Token_Equal_Equal] = BinaryOp_Eq,
        [Token_Bang_Equal]  = BinaryOp_Ne,
        [Token_Greater_Equal] = BinaryOp_Ge,
        [Token_Greater]     = BinaryOp_Gt,
    };
    BinaryOp op = bin_op_map[token];

    NodeId id = reserve_node(parser);
    advance(parser);

    ParseRule rule = rules[token];
    Node* right = precedence(parser, (Precedence)(rule.precedence + 1));

    NodeBinary binary = {
        node_base_binary(start, start),
        .op = op,
        .left = left,
        .right = right
    };

    return set_node(parser, id, node_binary(binary));
}


static Node* call(Parser* parser, Node* left) {
    assert(current(parser) == Token_Open_Paren && "Expected '(' token");
    assert(left->kind == NodeKind_Identifier && "Expected identifier node");

    TokenIndex start = parser->token_index;
    advance(parser);

    NodeId snapshot = stack_snapshot(parser);
    Node* node = NULL;
    {
        while (current(parser) != Token_Close_Paren && current(parser) != Token_Eof) {
            if ((node = expression(parser)) == NULL)
                return NULL;

            stack_push(parser, node);

            // If there is a comma, advance past it and continue parsing arguments.
            if (current(parser) == Token_Comma) {
                advance(parser);
                continue;
            } else {
                break;
            }
        }

        if (current(parser) != Token_Close_Paren) {
            fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Expected ')' after argument list, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int begin = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->tokens.source, begin, (int)start+1);
            return NULL;
        }

        // Advance past the ')' token.
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** expressions = stack_restore(parser, snapshot);

    NodeCall call = {
        node_base_call(start, node == NULL ? start : node->base.end),
        .name = left->identifier.name,
        .count = (i32) count,
        .args = expressions
    };
    return add_node(parser, node_call(call));
}


static Node* expression(Parser* parser) {
    return precedence(parser, Precedence_Assignment);
}


static Node* group(Parser* parser) {
    Token token = current(parser);
    assert(token == Token_Open_Paren && "Expected '(' token");
    advance(parser);

    Node* expr = expression(parser);
    if (expr == NULL)
        return NULL;

    token = current(parser);
    if (token != Token_Close_Paren) {
        fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Expected ')' after expression, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->tokens.source, start, start+1);
        return NULL;
    }
    advance(parser);

    return expr;
}

static Node* parse_type(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    const char* literal_type = NULL;
    if (strcmp(repr, "int") == 0) {
        literal_type = (const char*)(size_t)(LiteralType_Integer);
    } else if (strcmp(repr, "real") == 0) {
        literal_type = (const char*)(size_t)(LiteralType_Real);
    } else if (strcmp(repr, "str") == 0) {
        literal_type = (const char*)(size_t)(LiteralType_String);
    } else if (strcmp(repr, "void") == 0) {
        literal_type = (const char*)(size_t)(LiteralType_Void);
    } else {
        literal_type = repr;
    }

    NodeType type = {
        node_base_type(start, start),
        .name = literal_type,
    };
    return add_node(parser, node_type(type));

}

static Node* assign(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Equal) {
        fprintf(stderr, "Expected '=' after identifier\n");
        return NULL;
    }

    advance(parser);
    Node* expr = expression(parser);

    NodeAssign assign = {
        node_base_assign(start, expr->base.end),
        .name = repr,
        .expression = expr
    };
    return add_node(parser, node_assign(assign));
}

static Node* var_decl(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Colon_Equal) {
        fprintf(stderr, "Expected ':=' after identifier\n");
        return NULL;
    }

    advance(parser);
    Node* expr = expression(parser);
    if (expr == NULL)
        return NULL;

    NodeVarDecl var_decl = {
        node_base_var_decl(start, expr->base.end),
        .name = repr,
        .decl_offset = parser->current_decl_count++,
        .expression = expr
    };

    return add_node(parser, node_var_decl(var_decl));
}

static Node* block(Parser* parser) {
    assert(current(parser) == Token_Open_Brace && "Expected '{' token");

    TokenIndex start = parser->token_index;
    advance(parser);

    NodeBlock* block = (NodeBlock*) add_node(parser, node_block((NodeBlock) {
            node_base_block(start, 0),
            .id = (i32) ++parser->block_count,
            .parent = parser->current_block == NULL ? 0 : parser->current_block->id,
    }));
    NodeBlock* previous_block = parser->current_block;
    parser->current_block = block;

    size_t snapshot = stack_snapshot(parser);
    Node* node = NULL;
    {
        while (current(parser) != Token_Close_Brace && current(parser) != Token_Eof) {
            if ((node = statement(parser)) == NULL)
                return NULL;
            stack_push(parser, node);
        }
        if (current(parser) != Token_Close_Brace) {
            fprintf(stderr, "Expected '}' after block\n");
            return NULL;
        }
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** statements = stack_restore(parser, snapshot);

    TokenIndex stop = parser->token_index;
    parser->current_block->base = node_base_block(start, stop);
    parser->current_block->nodes = statements;
    parser->current_block->count = (i32) count;

    parser->current_block = previous_block;
    return (Node*) block;
}

static Node* fun_body(Parser* parser) {
    assert(current(parser) == Token_Open_Brace && "Expected '{' token");

    TokenIndex start = parser->token_index;
    advance(parser);

    NodeFunBody* body = (NodeFunBody*) add_node(parser, node_fun_body((NodeFunBody) {
            node_base_fun_body(start, 0),
            .id = (i32) ++parser->block_count,
            .parent = parser->current_block == NULL ? 0 : parser->current_block->id,
    }));

    NodeBlock* previous_block = parser->current_block;
    int previous_decl_count = parser->current_decl_count;

    parser->current_block = (NodeBlock*) body;

    size_t snapshot = stack_snapshot(parser);
    Node* node = NULL;
    {
        while (current(parser) != Token_Close_Brace && current(parser) != Token_Eof) {
            if ((node = statement(parser)) == NULL)
                return NULL;
            stack_push(parser, node);
        }
        if (current(parser) != Token_Close_Brace) {
            fprintf(stderr, "Expected '}' after block\n");
            return NULL;
        }
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** statements = stack_restore(parser, snapshot);

    body->decls = parser->current_decl_count - previous_decl_count;

    TokenIndex stop = parser->token_index;
    parser->current_block->base = node_base_fun_body(start, stop);
    parser->current_block->nodes = statements;
    parser->current_block->count = (i32) count;

    parser->current_block = previous_block;
    parser->current_decl_count = previous_decl_count;
    return (Node*) body;
}

static NodeFunParam* fun_param(Parser* parser, int offset) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Colon) {
        fprintf(stderr, "Expected ':' after identifier\n");
        return NULL;
    }
    advance(parser);

    if (current(parser) != Token_Identifier) {
        fprintf(stderr, "Expected type after ':'\n");
        return NULL;
    }
    Node* type = parse_type(parser);

    NodeFunParam fun_param = {
        node_base_fun_param(start, start),
        .offset = offset,
        .name = repr,
        .type = type,
    };
    return (NodeFunParam*) add_node(parser, node_fun_param(fun_param));
}

static NodeFunParam** fun_params(Parser* parser, size_t* count) {
    size_t snapshot = stack_snapshot(parser);
    NodeFunParam* param = NULL;
    {
        int offset = 0;
        while (current(parser) != Token_Close_Paren && current(parser) != Token_Eof) {
            if ((param = fun_param(parser, offset++)) == NULL)
                return NULL;
            stack_push(parser, (Node*) param);
            if (current(parser) == Token_Comma)
                advance(parser);
            else
                break;
        }
        if (current(parser) != Token_Close_Paren) {
            fprintf(stderr, "Expected ')' after argument list\n");
            return NULL;
        }
        advance(parser);
    }
    *count = parser->stack_count - snapshot;
    return (NodeFunParam**) stack_restore(parser, snapshot);
}

static NodeFunDecl* fun_decl(Parser* parser) {
    assert(current(parser) == Token_Fun && "Expected 'fun' token");
    TokenIndex start = parser->token_index;
    advance(parser);

    if (current(parser) != Token_Identifier) {
        fprintf(stderr, "Expected identifier after 'fun'\n");
        return NULL;
    }
    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Open_Paren) {
        fprintf(stderr, "Expected '(' after identifier\n");
        return NULL;
    }
    advance(parser);

    size_t count;
    NodeFunParam** params = fun_params(parser, &count);

    parser->current_decl_count = (int) count;

    Node* type = NULL;
    if (current(parser) != Token_Open_Brace) {
        type = parse_type(parser);
    }

    Node* body = fun_body(parser);
    if (body == NULL)
        return NULL;

    NodeFunDecl fun_decl = {
        .base = node_base_fun_decl(start, body->base.end),
        .name = repr,
        .params = params,
        .return_type = type,
        .param_count = (i32) count,
        .body = (NodeFunBody *) body,
    };
    return (NodeFunDecl*) add_node(parser, node_fun_decl(fun_decl));
}

static Node* return_stmt(Parser* parser) {
    assert(current(parser) == Token_Return && "Expected 'return' token");
    TokenIndex start = parser->token_index;
    advance(parser);

    // TODO(ted): Make a void/unit expression to disambiguate (is it needed?)
    Node* expr = expression(parser);
    if (expr == NULL)
        return NULL;

    NodeReturn return_stmt = {
        node_base_return_stmt(start, strlen("return")),
        .expression = expr
    };
    return add_node(parser, node_return_stmt(return_stmt));
}

static Node* if_stmt(Parser* parser) {
    assert(current(parser) == Token_If && "Expected 'if' token");
    TokenIndex start = parser->token_index;
    advance(parser);

    Node* condition = expression(parser);
    if (condition == NULL) {
        return NULL;
    }

    Node* then_block = block(parser);
    if (then_block == NULL) {
        return NULL;
    }

    Node* else_block = NULL;
    if (current(parser) == Token_Else) {
        advance(parser);
        else_block = block(parser);
    }

    NodeIf if_stmt = {
        node_base_if_stmt(start, then_block->base.end),
        .condition = condition,
        .then_block = (NodeBlock*) then_block,
        .else_block = (NodeBlock*) else_block
    };
    return add_node(parser, node_if_stmt(if_stmt));
}


static Node* while_stmt(Parser* parser) {
    assert(current(parser) == Token_While && "Expected 'while' token");
    TokenIndex start = parser->token_index;
    advance(parser);

    Node* condition = expression(parser);
    if (condition == NULL) {
        return NULL;
    }

    Node* then_block = block(parser);
    if (then_block == NULL) {
        return NULL;
    }

    Node* else_block = NULL;
    if (current(parser) == Token_Else) {
        advance(parser);
        else_block = block(parser);
    }

    NodeWhile while_stmt = {
        node_base_while_stmt(start, then_block->base.end),
        .condition = condition,
        .then_block = (NodeBlock*) then_block,
        .else_block = (NodeBlock*) else_block
    };
    return add_node(parser, node_while_stmt(while_stmt));
}


static Node* statement(Parser* parser) {
    Token token = current(parser);

    switch (token) {
        case Token_Plus:
        case Token_Asterisk:
        case Token_Equal:
        case Token_Minus:
        case Token_Slash:
        case Token_Percent:
        case Token_Less:
        case Token_Less_Equal:
        case Token_Equal_Equal:
        case Token_Bang_Equal:
        case Token_Greater_Equal:
        case Token_Greater:
        case Token_Bang:
        case Token_Colon_Equal:
        case Token_Close_Paren:
        case Token_Close_Brace:
        case Token_Colon:
        case Token_Comma:
        case Token_Else: {
            fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Invalid token: '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->tokens.source, start, start+1);
            return NULL;
        } break;
        case Token_Identifier: {
            if (peek(parser) == Token_Colon_Equal) {
                return var_decl(parser);
            } else if (peek(parser) == Token_Equal) {
                return assign(parser);
            } else {
                return expression(parser);
            }
        } break;
        case Token_Number:
        case Token_Real:
        case Token_String:
        case Token_Open_Paren: {
            return expression(parser);
        } break;
        case Token_Open_Brace: {
            return block(parser);
        } break;
        case Token_If: {
            return if_stmt(parser);
        } break;
        case Token_While: {
            return while_stmt(parser);
        } break;
        case Token_Fun: {
            return (Node*) fun_decl(parser);
        } break;
        case Token_Return: {
            return (Node*) return_stmt(parser);
        }
        case Token_Eof: {
            return NULL;
        }
    }
}


GrammarTree parse(const TokenArray tokens) {
    Parser parser = {
        .tokens = tokens,
        .token_index = 0,
        .stack = (Node**) malloc(1024 * sizeof(Node*)),
        .stack_count = 0,
        .current_block = NULL,
        .current_decl_count = 0,
        .block_count = 0,
        .nodes = (Node*) malloc(1024 * sizeof(Node)),
        .node_count = 0,
        .views = (Node**) malloc(1024 * sizeof(Node*)),
        .view_count = 0,
    };

    // Reserve one slot so that any references to 0 are invalid,
    // as no nodes should be able to reference a start node.
    TokenIndex first = parser.token_index;
    NodeId module_id = reserve_node(&parser);

    size_t snapshot = stack_snapshot(&parser);
    while (parser.token_index < tokens.size) {
        Token token = current(&parser);

        if (token != Token_Eof) {
            Node* node = statement(&parser);
            if (node == NULL) {
                goto error;
            }
            stack_push(&parser, node);
        } else {
            int count = (int) (parser.stack_count - snapshot);
            Node** statements = stack_restore(&parser, snapshot);

            int fun_count = sort_nodes_by_fun_decl(statements, count);
            int stmt_count = count - fun_count;

            TokenIndex stop = parser.token_index;
            NodeModule module = {
                node_base_module(first, stop),
                .stmts = statements + fun_count,
                .stmt_count = (i32) stmt_count,
                .decls = statements,
                .decl_count = fun_count,
                .global_count = parser.current_decl_count,
            };
            return parser_to_ast(&parser, (Node*) set_node(&parser, module_id, node_module(module)));
        }
    }

    fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Unexpected end of file\n", STR_ARG(tokens.name));
    
    error:;
    parser_free(&parser);
    return (GrammarTree) {tokens, NULL, NULL, NULL, 0 };
}


void grammar_tree_free(GrammarTree ast) {
    free(ast.nodes);
    free(ast.views);
    token_array_free(ast.tokens);
}
