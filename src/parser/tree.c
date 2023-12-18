#include "tree.h"

#include "parser.h"
#include "error.h"
#include "assert.h"
#include "os/memory.h"


GrammarTree parser_to_ast(Parser* parser, Node* start) {
    dealloc(parser->stack);
    return (GrammarTree) {
            parser->tokens,
            parser->nodes,
            start,
            parser->views,
            parser->block_count
    };
}

void grammar_tree_free(GrammarTree ast) {
    dealloc(ast.nodes);
    dealloc(ast.views);
}


// Sorts the nodes to put either function declarations or struct declarations first,
// while preserving the order of the other nodes.
static int sort_nodes_by_decl(Node** nodes, int count) {
    int fun_count = 0;
    for (int i = 0; i < count; ++i) {
        if (nodes[i]->kind == NodeKind_FunDecl || nodes[i]->kind == NodeKind_Struct) {
            for (int j = i; j > fun_count; --j) {
                if (nodes[j - 1]->kind != NodeKind_FunDecl && nodes[j - 1]->kind != NodeKind_Struct) {
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



static Node* literal(Parser*);
static Node* identifier(Parser*);
static Node* group(Parser*);

static Node* unary(Parser*);

static Node* binary(Parser* parser, Node* left);
static Node* call(Parser* parser, Node* left);
static Node* access(Parser* parser, Node* left);
static Node* init(Parser* parser, Node* left);
static Node* assign(Parser* parser, Node* left);

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

ParseRule rules[38] = {
        [Token_Number]              = { literal,      NULL,         Precedence_None},
        [Token_Real]                = { literal,      NULL,         Precedence_None},
        [Token_String]              = { literal,      NULL,         Precedence_None},
        [Token_Identifier]          = { identifier,   NULL,         Precedence_None},
        [Token_Minus]               = { unary,        binary,       Precedence_Term},
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
        [Token_Equal]               = { NULL,         assign,       Precedence_Assignment},
        [Token_Colon]               = { NULL,         NULL,         Precedence_None},
        [Token_Colon_Equal]         = { NULL,         NULL,         Precedence_None},
        [Token_Dot]                 = { NULL,         access,       Precedence_Call},
        [Token_True]                = { literal,      NULL,         Precedence_None},
        [Token_False]               = { literal,      NULL,         Precedence_None},
        [Token_Not]                 = { unary,        NULL,         Precedence_Unary},
        [Token_And]                 = { NULL,         binary,       Precedence_And},
        [Token_Or]                  = { NULL,         binary,       Precedence_Or},
        [Token_If]                  = { NULL,         NULL,         Precedence_None},
        [Token_Else]                = { NULL,         NULL,         Precedence_None},
        [Token_Then]                = { NULL,         NULL,         Precedence_None},
        [Token_While]               = { NULL,         NULL,         Precedence_None},
        [Token_Fun]                 = { NULL,         NULL,         Precedence_None},
        [Token_Return]              = { NULL,         NULL,         Precedence_None},
        [Token_Struct]              = { NULL,         NULL,         Precedence_None},
        [Token_Open_Paren]          = { group,        call,         Precedence_Call},
        [Token_Close_Paren]         = { NULL,         NULL,         Precedence_None},
        [Token_Open_Brace]          = { NULL,         init,         Precedence_Call},
        [Token_Close_Brace]         = { NULL,         NULL,         Precedence_None},
        [Token_Comma]               = { NULL,         NULL,         Precedence_None},
        [Token_Eof]                 = { NULL,         NULL,         Precedence_None},
};


static Node* precedence(Parser* parser, Precedence precedence) {
    Token token = current(parser);

    ParsePrefixFn prefix = rules[token].prefix;
    if (prefix == NULL) {
        error(parser->logger, STR_FMT "\n    Expected expression, got '%s\n'", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }

    Node* left = prefix(parser);
    if (left == NULL) {
        return NULL;
    }

    while (1) {
        token = current(parser);
        Precedence next_precedence = rules[token].precedence;
        if (precedence > next_precedence) {
            break;
        }

        // We temporarily don't allow brace initializers in expressions that is followed by a body.
        if (token == Token_Open_Brace && parser->is_in_expression_where_body_follows) {
            break;
        }

        ParseInfixFn infix = rules[token].infix;
        left = infix(parser, left);
        if (left == NULL) {
            return NULL;
        }
    }

    return left;
}

static Node* literal(Parser* parser) {
    Token token = current(parser);
    assert(((token_group(token) & TokenGroup_Literal) == TokenGroup_Literal) && "Expected literal token");
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);

    LiteralValue value;
    LiteralType  type;
    switch (token) {
        case Token_True:
            value.integer = 1;
            type = LiteralType_Boolean;
            break;
        case Token_False:
            value.integer = 0;
            type = LiteralType_Boolean;
            break;
        case Token_Number:
            value.integer = strtoll(repr, NULL, 10);
            type = LiteralType_Integer;
            break;
        case Token_Real:
            value.real = strtod(repr, NULL);
            type = LiteralType_Real;
            break;
        case Token_String:
            value.string = repr;
            type = LiteralType_String;
            break;
        default:
            assert(0 && "Unreachable");
    }
    advance(parser);

    NodeLiteral literal = { node_base_literal(start, start), .type = type, .value = value };
    return add_node(parser, node_literal(literal));
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


static Node* unary(Parser* parser) {
    Token token = current(parser);
    assert((
       ((token_group(token) & TokenGroup_Arithmetic_Operator) == TokenGroup_Arithmetic_Operator) ||
       ((token_group(token) & TokenGroup_Logical_Operator)    == TokenGroup_Logical_Operator)
    ) && "Expected unary operator");
    TokenIndex start = parser->token_index;

    static const UnaryOp unary_op_map[] = {
            [Token_Minus] = UnaryOp_Neg,
            [Token_Not]   = UnaryOp_Not,
    };
    UnaryOp op = unary_op_map[token];

    advance(parser);
    Node* expr = expression(parser);
    if (expr == NULL)
        return NULL;

    NodeUnary unary = {
            node_base_unary(start, expr->base.end),
            .expr = expr,
            .op = op,
    };
    return add_node(parser, node_unary(unary));
}

static Node* binary(Parser* parser, Node* left) {
    Token token = current(parser);
    assert((
       ((token_group(token) & TokenGroup_Arithmetic_Operator) == TokenGroup_Arithmetic_Operator) ||
       ((token_group(token) & TokenGroup_Comparison_Operator) == TokenGroup_Comparison_Operator) ||
       ((token_group(token) & TokenGroup_Logical_Operator) == TokenGroup_Logical_Operator)
    ) && "Expected binary operator");
    TokenIndex start = parser->token_index;

    static const BinaryOp bin_op_map[] = {
            [Token_Plus]            = BinaryOp_Add,
            [Token_Minus]           = BinaryOp_Sub,
            [Token_Asterisk]        = BinaryOp_Mul,
            [Token_Slash]           = BinaryOp_Div,
            [Token_Percent]         = BinaryOp_Mod,
            [Token_Less]            = BinaryOp_Lt,
            [Token_Less_Equal]      = BinaryOp_Le,
            [Token_Equal_Equal]     = BinaryOp_Eq,
            [Token_Bang_Equal]      = BinaryOp_Ne,
            [Token_Greater_Equal]   = BinaryOp_Ge,
            [Token_Greater]         = BinaryOp_Gt,
            [Token_And]             = BinaryOp_And,
            [Token_Or]              = BinaryOp_Or,
    };
    BinaryOp op = bin_op_map[token];

    NodeId id = reserve_node(parser);
    advance(parser);

    ParseRule rule = rules[token];
    Node* right = precedence(parser, (Precedence)(rule.precedence + 1));
    if (right == NULL) {
        return NULL;
    }

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
    if (left->kind != NodeKind_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier before '(' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }

    TokenIndex start = left->base.start;
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
            error(parser->logger, STR_FMT "\n    Expected ')' after argument list, got '%s\n'", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int begin = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, begin, (int)start+1);
            return NULL;
        }

        // Advance past the ')' token.
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** expressions = stack_restore(parser, snapshot);

    NodeCall call = {
            node_base_call(start, node == NULL ? left->base.end + 2 : node->base.end),
            .name = left->identifier.name,
            .count = (i32) count,
            .args = expressions
    };
    return add_node(parser, node_call(call));
}

static Node* access(Parser* parser, Node* left) {
    assert(current(parser) == Token_Dot && "Expected '.' token");

    // TODO: Temporary for now.
    if (left->kind != NodeKind_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier before '.' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }

    TokenIndex start = left->base.start;
    advance(parser);

    ParseRule rule = rules[Token_Dot];
    Node* right = precedence(parser, (Precedence)(rule.precedence + 1));
    if (right == NULL)
        return NULL;

    NodeAccess access = {
            node_base_access(start, start),
            .left = left,
            .right = right,
    };
    return add_node(parser, node_access(access));
}


static Node* init_arg(Parser* parser, int offset) {
    TokenIndex start = parser->token_index;

    const char* repr = NULL;
    if (current(parser) == Token_Identifier) {
        repr = repr_of_current(parser);
        advance(parser);
        if (current(parser) != Token_Equal) {
            error(parser->logger, STR_FMT "\n    Expected '=' after identifier, got '%s\n'", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start_ = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
            return NULL;
        }
        advance(parser);
    }

    Node* expr = expression(parser);
    if (expr == NULL)
        return NULL;

    NodeInitArg init_arg = {
            node_base_init_arg(start, expr->base.end),
            .name = repr,
            .offset = offset,
            .expr = expr
    };
    return add_node(parser, node_init_arg(init_arg));
}

static Node* init(Parser* parser, Node* left) {
    assert(current(parser) == Token_Open_Brace && "Expected '{' token");
    if (left->kind != NodeKind_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier before '{' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }

    TokenIndex start = left->base.start;
    advance(parser);

    NodeId snapshot = stack_snapshot(parser);
    Node* node = NULL;
    {
        int offset = 0;
        while (current(parser) != Token_Close_Brace && current(parser) != Token_Eof) {
            if ((node = init_arg(parser, offset++)) == NULL)
                return NULL;

            stack_push(parser, node);

            // TODO: Optional comma separator.
        }

        if (current(parser) != Token_Close_Brace) {
            error(parser->logger, STR_FMT "\n    Expected '}' after argument list, got '%s\n'", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int begin = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, begin, (int)start+1);
            return NULL;
        }

        // Advance past the '}' token.
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** expressions = stack_restore(parser, snapshot);

    NodeInit init = {
            node_base_init(start, node == NULL ? left->base.end + 2 : node->base.end),
            .name = left->identifier.name,
            .count = (i32) count,
            .args = (NodeInitArg**) expressions
    };
    return add_node(parser, node_init(init));
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
        error(parser->logger, STR_FMT "\n    Expected ')' after expression, got '%s\n'", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start, start+1);
        return NULL;
    }
    advance(parser);

    return expr;
}

static Node* parse_type(Parser* parser) {
    if (current(parser) != Token_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected type identifier, got '%s\n'", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start, start+1);
        return NULL;
    }
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

static Node* assign(Parser* parser, Node* left) {
    assert(current(parser) == Token_Equal && "Expected '=' token");
    // NOTE(ted): Temporary for now.
    if (left->kind != NodeKind_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier before '=' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }
    TokenIndex start = parser->token_index;

    advance(parser);
    Node* expr = expression(parser);
    if (expr == NULL)
        return NULL;

    NodeAssign assign = {
            node_base_assign(start, expr->base.end),
            .name = left->identifier.name,
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
        error(parser->logger, STR_FMT "\n    Expected ':=' after identifier, got '%s\n'", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
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
    if (current(parser) != Token_Open_Brace) {
        error(parser->logger, STR_FMT "\n    Expected '{' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }

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
            error(parser->logger, STR_FMT "\n    Expected '}' token after block, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start_ = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
            return NULL;
        }
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** statements = stack_restore(parser, snapshot);

    int decl_count = sort_nodes_by_decl(statements, count);

    TokenIndex stop = parser->token_index;
    parser->current_block->base = node_base_block(start, stop);
    parser->current_block->nodes = statements + decl_count;
    parser->current_block->count = (i32) count - (i32) decl_count;
    parser->current_block->decls = statements;
    parser->current_block->decl_count = decl_count;

    parser->current_block = previous_block;
    return (Node*) block;
}

static Node* fun_body(Parser* parser) {
    if (current(parser) != Token_Open_Brace) {
        error(parser->logger, STR_FMT "\n    Expected '{' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }

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
            error(parser->logger, STR_FMT "\n    Expected '}' token after block, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start_ = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
            return NULL;
        }
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** statements = stack_restore(parser, snapshot);

    int decl_count = sort_nodes_by_decl(statements, count);

    TokenIndex stop = parser->token_index;
    parser->current_block->base = node_base_block(start, stop);
    parser->current_block->nodes = statements;// + decl_count;
    parser->current_block->count = (i32) count;// - (i32) decl_count;
    parser->current_block->decls = statements;
    parser->current_block->decl_count = decl_count;

    parser->current_block = previous_block;
    parser->current_decl_count = previous_decl_count;
    return (Node*) body;
}

static NodeFunParam* fun_param(Parser* parser, int offset) {
    if (current(parser) != Token_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier in fun param, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Colon) {
        error(parser->logger, STR_FMT "\n    Expected ':' after identifier, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }
    advance(parser);

    if (current(parser) != Token_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected type after ':', got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }
    Node* type = parse_type(parser);
    if (type == NULL)
        return NULL;

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
    NodeFunParam* param;
    {
        int offset = 0;
        while (current(parser) != Token_Close_Paren && current(parser) != Token_Eof) {
            if ((param = fun_param(parser, offset++)) == NULL)
                return (NodeFunParam **) (-1);
            stack_push(parser, (Node*) param);
            if (current(parser) == Token_Comma)
                advance(parser);
            else
                break;
        }
        if (current(parser) != Token_Close_Paren) {
            error(parser->logger, STR_FMT "\n    Expected ')' after argument list, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start_ = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
            return (NodeFunParam **) (-1);
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

    int previous_decl_count = parser->current_decl_count;

    if (current(parser) != Token_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier after 'fun' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }
    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Open_Paren) {
        error(parser->logger, STR_FMT "\n    Expected '(' after identifier, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }
    advance(parser);

    size_t count;
    NodeFunParam** params = fun_params(parser, &count);
    if (params == (NodeFunParam**) (-1))
        return NULL;

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

    parser->current_decl_count = previous_decl_count;
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

    parser->is_in_expression_where_body_follows = 1;
    Node* condition = expression(parser);
    if (condition == NULL) {
        return NULL;
    }
    parser->is_in_expression_where_body_follows = 0;

    int expect_then_statement = 0;
    Node* then_block;
    if (current(parser) == Token_Then) {
        advance(parser);
        then_block = statement(parser);
        expect_then_statement = 1;
    } else {
        then_block = block(parser);
    }

    if (then_block == NULL) {
        return NULL;
    }

    Node* else_block = NULL;
    if (current(parser) == Token_Else) {
        advance(parser);
        if (current(parser) == Token_If) {
            else_block = if_stmt(parser);
        } else if (expect_then_statement) {
            else_block = statement(parser);
        } else {
            else_block = block(parser);
        }
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

    parser->is_in_expression_where_body_follows = 1;
    Node* condition = expression(parser);
    if (condition == NULL) {
        return NULL;
    }
    parser->is_in_expression_where_body_follows = 0;

    Node* then_block = block(parser);
    if (then_block == NULL) {
        return NULL;
    }

    Node* else_block = NULL;
    /* Not implemented
    if (current(parser) == Token_Else) {
        advance(parser);
        else_block = block(parser);
    }
    */

    NodeWhile while_stmt = {
            node_base_while_stmt(start, then_block->base.end),
            .condition = condition,
            .then_block = (NodeBlock*) then_block,
            .else_block = (NodeBlock*) else_block
    };
    return add_node(parser, node_while_stmt(while_stmt));
}

static Node* struct_field(Parser* parser) {
    if (current(parser) != Token_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier in struct field, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        return NULL;
    }
    TokenIndex start = parser->token_index;

    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Colon) {
        error(parser->logger, STR_FMT "\n    Expected ':' after identifier in struct field, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        return NULL;
    }
    advance(parser);

    Node* type = parse_type(parser);
    if (type == NULL)
        return NULL;

    Node* expr = NULL;
    if (current(parser) == Token_Equal) {
        advance(parser);
        expr = expression(parser);
        if (expr == NULL)
            return NULL;
    }

    NodeStructField struct_field = {
            node_base_struct_field(start, type->base.end),
            .name = repr,
            .type = type,
            .expr = expr,
            .offset = parser->struct_field_offset++,
    };
    return add_node(parser, node_struct_field(struct_field));
}


static Node* struct_decl(Parser* parser) {
    assert(current(parser) == Token_Struct && "Expected 'struct' token");
    TokenIndex start = parser->token_index;
    advance(parser);

    if (current(parser) != Token_Identifier) {
        error(parser->logger, STR_FMT "\n    Expected identifier after 'struct' token, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_, start_+1);
        return NULL;
    }
    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Open_Brace) {
        error(parser->logger, STR_FMT "\n    Expected '{' after identifier, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
        int start_ = (int) parser->tokens.source_offsets[parser->token_index];
        point_to_error(parser->logger, parser->tokens.source, start_+1, start_+2);
        return NULL;
    }
    advance(parser);


    NodeStruct* body = (NodeStruct*) add_node(parser, node_struct_decl((NodeStruct) {
            node_base_struct_decl(start, 0),
            .id = (i32) ++parser->block_count,
            .parent = parser->current_block == NULL ? 0 : parser->current_block->id,
            .name = repr,
    }));

    NodeBlock* previous_block = parser->current_block;
    int previous_decl_count = parser->current_decl_count;

    parser->current_block = (NodeBlock*) body;



    int old_struct_field_offset = parser->struct_field_offset;
    parser->struct_field_offset = 0;

    size_t snapshot = stack_snapshot(parser);
    Node* node = NULL;
    {
        while (current(parser) != Token_Close_Brace && current(parser) != Token_Eof) {
            if ((node = struct_field(parser)) == NULL)
                return NULL;
            stack_push(parser, node);
        }
        if (current(parser) != Token_Close_Brace) {
            error(parser->logger, STR_FMT "\n    Expected '}' token after block, got '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start_ = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, start_+1, start_+2);
            return NULL;
        }
        advance(parser);
    }
    size_t count = parser->stack_count - snapshot;
    Node** fields = stack_restore(parser, snapshot);

    body->count = parser->current_decl_count - previous_decl_count;

    TokenIndex stop = parser->token_index;
    parser->current_block->base = node_base_struct_decl(start, stop);
    parser->current_block->nodes = fields;
    parser->current_block->count = (i32) count;

    parser->struct_field_offset = old_struct_field_offset;

    parser->current_block = previous_block;
    parser->current_decl_count = previous_decl_count;
    return (Node*) body;
}

static Node* statement(Parser* parser) {
    Token token = current(parser);

    static_assert(TOKEN_LAST == 37, "Expected to handle this many tokens. Token has been updated!");
    switch (token) {
        case Token_Plus:
        case Token_Asterisk:
        case Token_Equal:
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
        case Token_Dot:
        case Token_And:
        case Token_Or:
        case Token_Else:
        case Token_Then: {
            error(parser->logger, STR_FMT "\n    Unexpected token '%s'\n", STR_ARG(parser->tokens.name), lexer_repr_of(parser->tokens, parser->token_index));
            int start = (int) parser->tokens.source_offsets[parser->token_index];
            point_to_error(parser->logger, parser->tokens.source, start, start+1);
            return NULL;
        } break;
        case Token_Identifier: {
            if (peek(parser) == Token_Colon_Equal) {
                return var_decl(parser);
            } else {
                return expression(parser);
            }
        } break;
        case Token_True:
        case Token_False:
        case Token_Number:
        case Token_Real:
        case Token_String:
        case Token_Minus:
        case Token_Not:
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
        case Token_Struct: {
            return (Node*) struct_decl(parser);
        }
        case Token_Eof: {
            return NULL;
        }
    }
}


GrammarTree parse(TokenArray tokens, Logger* logger) {
    Parser parser = {
            .logger = logger,
            .tokens = tokens,
            .token_index = 0,
            .stack = (Node**) alloc(1024 * sizeof(Node*)),
            .stack_count = 0,
            .current_block = NULL,
            .current_decl_count = 0,
            .block_count = 0,
            .nodes = (Node*) alloc(1024 * sizeof(Node)),
            .node_count = 0,
            .views = (Node**) alloc(1024 * sizeof(Node*)),
            .view_count = 0,
            .is_in_expression_where_body_follows = 0
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

            int fun_count = sort_nodes_by_decl(statements, count);
            int stmt_count = count - fun_count;

            TokenIndex stop = parser.token_index;
            NodeModule module = {
                    node_base_module(first, stop),
                    .stmts = statements + fun_count,  // TODO: statements might be NULL.
                    .stmt_count = (i32) stmt_count,
                    .decls = statements,
                    .decl_count = fun_count,
                    .global_count = parser.current_decl_count,
            };
            return parser_to_ast(&parser, (Node*) set_node(&parser, module_id, node_module(module)));
        }
    }

    error(parser.logger, STR_FMT "\n    Unexpected end of file", STR_ARG(tokens.name));

    error:;
    parser_free(&parser);
    return (GrammarTree) {tokens, NULL, NULL, NULL, 0 };
}


