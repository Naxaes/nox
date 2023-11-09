#include "parser.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct {
    TokenArray tokens;
    TokenId    current_token_id;
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

static inline Token current(Parser* parser) {
    return parser->tokens.tokens[parser->current_token_id];
}

static inline Token peek(Parser* parser) {
    if (parser->current_token_id + 1 >= parser->tokens.size)
        return Token_Eof;
    return parser->tokens.tokens[parser->current_token_id + 1];
}

static inline TokenId advance(Parser* parser) {
    return parser->current_token_id++;
}

static inline const char* repr_of_current(Parser* parser) {
    return lexer_repr_of(parser->tokens, parser->current_token_id);
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


static Node* number(Parser*);
static Node* real(Parser*);
static Node* identifier(Parser*);
static Node* binary(Parser* parser, Node* left);


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
        [Token_Invalid]     =   { NULL,         NULL,         Precedence_None},
        [Token_Number]      =   { number,       NULL,         Precedence_None},
        [Token_Real]        =   { real,         NULL,         Precedence_None},
        [Token_Identifier]  =   { identifier,   NULL,         Precedence_None},
        [Token_Plus]        =   { NULL,         binary,       Precedence_Term},
        [Token_Asterisk]    =   { NULL,         binary,       Precedence_Factor},
        [Token_Equal]       =   { NULL,         NULL,         Precedence_None},
        [Token_Eof]         =   { NULL,         NULL,         Precedence_None},
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
    TokenId start = parser->current_token_id;

    const char* repr = repr_of_current(parser);
    i64 value = strtoll(repr, NULL, 10);
    advance(parser);

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = Literal_Integer, .value.integer = value };
    return add_node(parser, (Node) { .literal = literal });
}

static Node* real(Parser* parser) {
    assert(current(parser) == Token_Real && "Expected real token");
    TokenId start = parser->current_token_id;

    const char* repr = repr_of_current(parser);
    f64 value = strtod(repr, NULL);
    advance(parser);

    NodeLiteral literal = { { NodeKind_Literal, start, start }, .type = Literal_Real, .value.real = value };
    return add_node(parser, (Node) { .literal = literal });
}

static Node* identifier(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");
    TokenId start = parser->current_token_id;

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
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op='+' };
        } break;
        case Token_Asterisk: {
            binary = (NodeBinary) { { NodeKind_Binary, left->base.start, 0 }, .left=left, .right=0, .op='*' };
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


static Node* expression(Parser* parser) {
    return precedence(parser, Precedence_Assignment);
}


static Node* assignment(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected number token");
    TokenId start = parser->current_token_id;

    const char* repr = repr_of_current(parser);
    advance(parser);

    if (current(parser) != Token_Equal) {
        fprintf(stderr, "Expected '=' after identifier\n");
        return NULL;
    }

    advance(parser);
    Node* expr = expression(parser);

    NodeVarDecl var_decl = { { NodeKind_VarDecl, start, expr->base.end }, .name = repr, .expression = expr };
    return add_node(parser, (Node) { .var_decl = var_decl });
}


UntypedAst parse(TokenArray tokens) {
    Parser parser = {
        .tokens = tokens,
        .current_token_id = 0,
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

    TokenId first = parser.current_token_id;

    while (parser.current_token_id < tokens.size) {
        Token token = current(&parser);

        switch (token) {
            case Token_Plus:
            case Token_Asterisk:
            case Token_Equal:
            case Token_Invalid: {
                fprintf(stderr, "[Error] (Parser) " STR_FMT "\n    Invalid token: '%s'\n", STR_ARG(tokens.name), lexer_repr_of(tokens, parser.current_token_id));
                int start = (int) tokens.indices[parser.current_token_id];
                point_to_error(tokens.source, start, start+1);
                goto error;
            } break;
            case Token_Identifier: {
                if (peek(&parser) == Token_Equal) {
                    node = assignment(&parser);
                } else {
                    node = expression(&parser);
                }
            } break;
            case Token_Number:
            case Token_Real: {
                node = expression(&parser);
            } break;
            case Token_Eof: {
                for (size_t i = 0; i < parser.stack_count; ++i) {
                    parser.views[parser.view_count++] = parser.stack[i];
                }
                parser.views[parser.view_count++] = 0;

                TokenId stop = parser.current_token_id;
                Node** statements = parser.views + (parser.view_count - parser.stack_count - 1);
                NodeBlock block = { { NodeKind_Block, first, stop }, .nodes = statements };

                parser.stack_count = 0;
                parser.view_count += parser.stack_count + 1;

                node = add_node(&parser, (Node) { .block = block });

                return parser_to_ast(&parser, node);
            }
        }

        if (node == 0) {
            goto error;
        }

        parser.stack[parser.stack_count++] = node;
    }

    fprintf(stderr, "Unexpected end of token stream\n");
    error:;
    parser_free(&parser);
    return (UntypedAst) { tokens, NULL, NULL, NULL };
}


void grammar_tree_free(UntypedAst ast) {
    free(ast.nodes);
    free(ast.views);
    token_array_free(ast.tokens);
}
