#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct {
    TokenArray tokens;
    TokenId    current_token;
    Node*      nodes;
    NodeId     node_count;
} Parser;

static inline Token current(Parser* parser) {
    return parser->tokens.tokens[parser->current_token];
}

static inline Token peek(Parser* parser) {
    if (parser->current_token + 1 >= parser->tokens.size)
        return Token_Eof;
    return parser->tokens.tokens[parser->current_token + 1];
}

static inline TokenId advance(Parser* parser) {
    return parser->current_token++;
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
        [Token_Identifier]  =   { NULL,         NULL,         Precedence_None},
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

    const char* repr = lexer_repr_of(parser->tokens, parser->current_token);
    i64 value = strtoll(repr, NULL, 10);
    advance(parser);

    NodeLiteral literal = { NodeKind_Literal, .type=Literal_Integer, .value.integer = value };
    return add_node(parser, (Node) { .literal = literal });
}

static Node* binary(Parser* parser, Node* left) {
    Token token = current(parser);
    NodeBinary binary;

    switch (token) {
        case Token_Plus: {
            binary = (NodeBinary) { NodeKind_Binary, .left=left, .right=0, .op='+' };
        } break;
        case Token_Asterisk: {
            binary = (NodeBinary) { NodeKind_Binary, .left=left, .right=0, .op='*' };
        } break;
        default: {
            fprintf(stderr, "Unexpected path\n");
            return NULL;
        }
    }

    NodeId id = reserve_node(parser);
    advance(parser);

    ParseRule rule = rules[token];
    binary.right = precedence(parser, (Precedence)(rule.precedence + 1));

    return set_node(parser, id, (Node) { .binary = binary });
}


static Node* expression(Parser* parser) {
    return precedence(parser, Precedence_Assignment);
}


static Node* assignment(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected number token");

    const char* repr = lexer_repr_of(parser->tokens, parser->current_token);
    advance(parser);

    if (current(parser) != Token_Equal) {
        fprintf(stderr, "Expected '=' after identifier\n");
        return NULL;
    }

    advance(parser);
    Node* expr = expression(parser);

    NodeVarDecl var_decl = { NodeKind_VarDecl, .name = repr, .expression = expr };
    return add_node(parser, (Node) { .var_decl = var_decl });
}


UntypedAst parse(TokenArray tokens) {
    Parser parser = {
        .tokens = tokens,
        .current_token = 0,
        .nodes = (Node*) malloc(1024),
        .node_count = 0,
    };

    // Reserve one slot so that any references to 0 are invalid,
    // as no nodes should be able to reference a start node.
    add_node(&parser, (Node) { NodeKind_Invalid });
    Node* node = 0;

    while (parser.current_token < tokens.size) {
        Token token = current(&parser);

        switch (token) {
            case Token_Plus:
            case Token_Asterisk:
            case Token_Equal:
            case Token_Invalid: {
                fprintf(stderr, "Invalid token: '%d'\n", token);
                free(parser.nodes);
                return (UntypedAst) { NULL, 0 };
            } break;
            case Token_Identifier: {
                if (peek(&parser) == Token_Equal) {
                    node = assignment(&parser);
                } else {
                    assert("TODO");
                }
            } break;
            case Token_Number: {
                node = expression(&parser);
            } break;
            case Token_Eof: {
                return (UntypedAst) {parser.nodes, node };
            }
        }

        if (node == 0)
            return (UntypedAst) { NULL, 0 };
    }

    fprintf(stderr, "Unexpected end of token stream\n");
    free(parser.nodes);
    return (UntypedAst) { NULL, 0 };
}
