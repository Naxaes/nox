#include "parser.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


const char* literal_type_name(enum LiteralType type) {
    static const char* TYPE_NAMES[] = {
        [Literal_Boolean] = "bool",
        [Literal_Integer] = "int",
        [Literal_Real]    = "real",
        [Literal_String]  = "string",
    };
    return TYPE_NAMES[type];
}


/* ---------------------------- NODE HELPERS -------------------------------- */
int node_is_expression(const Node* node) {
    switch (node->kind) {
        case NodeKind_Literal:
        case NodeKind_Identifier:
        case NodeKind_Call:
        case NodeKind_Binary:
            return 1;
        default:
            return 0;
    }
}

int node_is_statement(const Node* node) {
    return node->base.kind != NodeKind_Invalid;
}


/* ---------------------------- BINARY OP HELPERS -------------------------------- */
const char* binary_op_repr(NodeBinary binary) {
    switch (binary.op) {
        case Binary_Operation_Add:  return "+";
        case Binary_Operation_Sub:  return "-";
        case Binary_Operation_Mul:  return "*";
        case Binary_Operation_Div:  return "/";
        case Binary_Operation_Mod:  return "%";
        case Binary_Operation_Lt:   return "<";
        case Binary_Operation_Le:   return "<=";
        case Binary_Operation_Eq:   return "==";
        case Binary_Operation_Ne:   return "!=";
        case Binary_Operation_Ge:   return ">=";
        case Binary_Operation_Gt:   return ">";
        default:
            assert(0 && "Invalid binary operation");
    }
    assert(0 && "Invalid binary operation");
    return NULL;
}

int binary_op_is_arithmetic(NodeBinary binary) {
    switch (binary.op) {
        case Binary_Operation_Add:
        case Binary_Operation_Sub:
        case Binary_Operation_Mul:
        case Binary_Operation_Div:
        case Binary_Operation_Mod:
            return 1;
        default:
            return 0;
    }
}

int binary_op_is_comparison(NodeBinary binary) {
    switch (binary.op) {
        case Binary_Operation_Lt:
        case Binary_Operation_Le:
        case Binary_Operation_Eq:
        case Binary_Operation_Ne:
        case Binary_Operation_Ge:
        case Binary_Operation_Gt:
            return 1;
        default:
            return 0;
    }
}


/* ---------------------------- PARSER IMPL -------------------------------- */
typedef struct {
    TokenArray tokens;
    TokenIndex token_index;
    Node*      nodes;
    NodeId     node_count;
    Node**     views;
    NodeId     view_count;
    Node**     stack;
    NodeId     stack_count;
} Parser;

void parser_free(Parser* parser) {
    free(parser->nodes);
    free(parser->views);
    free(parser->stack);
    token_array_free(parser->tokens);
}

UntypedAst parser_to_ast(Parser* parser, Node* start) {
    free(parser->stack);
    return (UntypedAst) {
        parser->tokens,
        parser->nodes,
        start,
        parser->views
    };
}

/* ---------------------------- PARSER HELPERS -------------------------------- */
static inline Token current(Parser* parser) {
    return parser->tokens.tokens[parser->token_index];
}

static inline Token peek(Parser* parser) {
    if (parser->token_index + 1 >= parser->tokens.size)
        return Token_Eof;
    return parser->tokens.tokens[parser->token_index + 1];
}

static inline TokenIndex advance(Parser* parser) {
    return parser->token_index++;
}

static inline const char* repr_of_current(Parser* parser) {
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

static inline NodeId stack_snapshot(Parser* parser) {
    return parser->stack_count;
}

static inline void stack_push(Parser* parser, Node* node) {
    parser->stack[parser->stack_count++] = node;
}

static inline Node** stack_restore(Parser* parser, NodeId snapshot) {
    /*if (snapshot == parser->stack_count) {
        static Node** empty = NULL;
        return empty;
    }*/

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

ParseRule rules[] = {
        [Token_Invalid]             = { NULL,         NULL,         Precedence_None},
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
        [Token_Exclamation_Equal]   = { NULL,         binary,       Precedence_Equality},
        [Token_Greater_Equal]       = { NULL,         binary,       Precedence_Comparison},
        [Token_Greater]             = { NULL,         binary,       Precedence_Comparison},
        [Token_Exclamation]         = { NULL,         NULL,         Precedence_None},
        [Token_Equal]               = { NULL,         NULL,         Precedence_None},
        [Token_If]                  = { NULL,         NULL,         Precedence_None},
        [Token_Open_Paren]          = { group,        call,         Precedence_Call},
        [Token_Close_Paren]         = { NULL,         NULL,         Precedence_None},
        [Token_Open_Brace]          = { NULL,         NULL,         Precedence_None},
        [Token_Close_Brace]         = { NULL,         NULL,         Precedence_None},
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

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = Literal_Integer, .value.integer = value };
    return add_node(parser, (Node) { .literal = literal });
}

static Node* real(Parser* parser) {
    assert(current(parser) == Token_Real && "Expected real token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    f64 value = strtod(repr, NULL);
    advance(parser);

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = Literal_Real, .value.real = value };
    return add_node(parser, (Node) { .literal = literal });
}

static Node* string(Parser* parser) {
    assert(current(parser) == Token_String && "Expected string token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = Literal_String, .value.string = repr };
    return add_node(parser, (Node) { .literal = literal });
}

static Node* identifier(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    NodeIdentifier ident = { { NodeKind_Identifier, start, start }, .name = repr };
    return add_node(parser, (Node) { .identifier = ident });
}

static Node* binary(Parser* parser, Node* left) {
    Token token = current(parser);
    assert(token_is_binary_operator(token) && "Expected binary operator");

    NodeBinary binary;
    switch (token) {
        case Token_Plus: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Add };
        } break;
        case Token_Minus: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Sub };
        } break;
        case Token_Asterisk: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Mul };
        } break;
        case Token_Slash: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Div };
        } break;
        case Token_Percent: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Mod };
        } break;
        case Token_Less: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Lt };
        } break;
        case Token_Less_Equal: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Le };
        } break;
        case Token_Equal_Equal: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Eq };
        } break;
        case Token_Exclamation_Equal: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Ne };
        } break;
        case Token_Greater_Equal: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Ge };
        } break;
        case Token_Greater: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op=Binary_Operation_Gt };
        } break;
        default: {
            assert(0 && "Invalid binary operator");
        }
    }

    NodeId id = reserve_node(parser);
    advance(parser);

    ParseRule rule = rules[token];
    binary.right = precedence(parser, (Precedence)(rule.precedence + 1));
    binary.base.end = binary.right->base.end;

    return set_node(parser, id, (Node) { .binary = binary });
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
            parser->stack[parser->stack_count++] = node;
            if (current(parser) == Token_Comma)
                advance(parser);
            else
                break;
        }
        if (current(parser) != Token_Close_Paren) {
            fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Expected ')' after expression, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int begin = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->tokens.source, begin, (int)start+1);
            return NULL;
        }
        advance(parser);
    }

    Node** expressions = stack_restore(parser, snapshot);

    NodeCall call = { { NodeKind_Call, start, node == NULL ? start : node->base.end }, .name = left->identifier.name, .args = expressions };
    return add_node(parser, (Node) { .call = call });
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


static Node* assignment(Parser* parser) {
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

    NodeAssign assign = { { NodeKind_Assign, start, expr->base.end }, .name = repr, .expression = expr };
    return add_node(parser, (Node) { .assign = assign });
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

    NodeVarDecl var_decl = { { NodeKind_VarDecl, start, expr->base.end }, .name = repr, .expression = expr };
    return add_node(parser, (Node) { .var_decl = var_decl });
}

static Node* block(Parser* parser) {
    assert(current(parser) == Token_Open_Brace && "Expected '{' token");
    TokenIndex start = parser->token_index;
    advance(parser);

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
    Node** statements = stack_restore(parser, snapshot);

    NodeBlock block = { { NodeKind_Block, start, node->base.end }, .nodes = statements };
    return add_node(parser, (Node) { .block = block });
}

static NodeFunParam* fun_param(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    NodeFunParam fun_param = { { NodeKind_FunParam, start, start }, .name = repr };
    return (NodeFunParam*) add_node(parser, (Node) { .fun_param = fun_param });
}

static NodeFunParam** fun_params(Parser* parser) {
    size_t snapshot = stack_snapshot(parser);
    NodeFunParam* param = NULL;
    {
        while (current(parser) != Token_Close_Paren && current(parser) != Token_Eof) {
            if ((param = fun_param(parser)) == NULL)
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
    return (NodeFunParam**) stack_restore(parser, snapshot);
}

static Node* fun_decl(Parser* parser) {
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

    NodeFunParam** params = fun_params(parser);

    Node* body = block(parser);
    if (body == NULL)
        return NULL;

    NodeFunDecl fun_decl = { { NodeKind_FunDecl, start, body->base.end }, .name = repr, .params = params, .block = (NodeBlock*) body };
    return add_node(parser, (Node) { .fun_decl = fun_decl });
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

    NodeIf if_stmt = { { NodeKind_If, start, then_block->base.end }, .condition = condition, .then_block = (NodeBlock*) then_block, .else_block = (NodeBlock*) else_block };
    return add_node(parser, (Node) { .if_stmt = if_stmt });
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

    NodeWhile while_stmt = { { NodeKind_While, start, then_block->base.end }, .condition = condition, .then_block = (NodeBlock*) then_block, .else_block = (NodeBlock*) else_block };
    return add_node(parser, (Node) { .while_stmt = while_stmt });
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
        case Token_Exclamation_Equal:
        case Token_Greater_Equal:
        case Token_Greater:
        case Token_Exclamation:
        case Token_Colon_Equal:
        case Token_Close_Paren:
        case Token_Close_Brace:
        case Token_Colon:
        case Token_Comma:
        case Token_Else:
        case Token_Invalid: {
            fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Invalid token: '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->tokens.source, start, start+1);
            return NULL;
        } break;
        case Token_Identifier: {
            if (peek(parser) == Token_Colon_Equal) {
                return var_decl(parser);
            } else if (peek(parser) == Token_Equal) {
                return assignment(parser);
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
            return fun_decl(parser);
        } break;
        case Token_Eof: {
            return NULL;
        }
    }
}


UntypedAst parse(TokenArray tokens) {
    Parser parser = {
        .tokens = tokens,
        .token_index = 0,
        .nodes = (Node*) malloc(1024),
        .node_count = 0,
        .views = (Node**) malloc(1024),
        .view_count = 0,
        .stack = (Node**) malloc(1024),
        .stack_count = 0,
    };

    // Reserve one slot so that any references to 0 are invalid,
    // as no nodes should be able to reference a start node.
    reserve_node(&parser);
    Node* node = 0;

    TokenIndex first = parser.token_index;

    while (parser.token_index < tokens.size) {
        Token token = current(&parser);

        if (token != Token_Eof) {
            node = statement(&parser);
            if (node == NULL) {
                goto error;
            }
           parser.stack[parser.stack_count++] = node;
        } else {
            Node** statements = parser.views + parser.view_count;
            for (size_t i = 0; i < parser.stack_count; ++i) {
                parser.views[parser.view_count++] = parser.stack[i];
            }
            parser.views[parser.view_count++] = NULL;

            TokenIndex stop = parser.token_index;
            NodeBlock block = { { NodeKind_Block, first, stop }, .nodes = statements };

            node = add_node(&parser, (Node) { .block = block });
            return parser_to_ast(&parser, node);
        }
    }

    fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Unexpected end of file\n", STR_ARG(tokens.name));
    
    error:;
    parser_free(&parser);
    return (UntypedAst) { tokens, NULL, NULL, NULL };
}


void grammar_tree_free(UntypedAst ast) {
    free(ast.nodes);
    free(ast.views);
    token_array_free(ast.tokens);
}
