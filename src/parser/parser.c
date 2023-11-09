#include "parser.h"

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

    const char* repr = repr_of_current(parser);
    i64 value = strtoll(repr, NULL, 10);
    advance(parser);

    NodeLiteral literal = { NodeKind_Literal, .type = Literal_Integer, .value.integer = value };
    return add_node(parser, (Node) { .literal = literal });
}

static Node* identifier(Parser* parser) {
    assert(current(parser) == Token_Identifier && "Expected identifier token");

    const char* repr = repr_of_current(parser);
    advance(parser);

    NodeIdentifier ident = { NodeKind_Identifier, .name = repr };
    return add_node(parser, (Node) { .identifier = ident });
}

static Node* binary(Parser* parser, Node* left) {
    Token token = current(parser);
    assert(token_is_binary_operator(token) && "Expected binary operator");

    NodeBinary binary;
    switch (token) {
        case Token_Plus: {
            binary = (NodeBinary) { NodeKind_Binary, .left=left, .right=0, .op='+' };
        } break;
        case Token_Asterisk: {
            binary = (NodeBinary) { NodeKind_Binary, .left=left, .right=0, .op='*' };
        } break;
        default: {
            assert(0 && "Invalid binary operator");
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

    const char* repr = repr_of_current(parser);
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

    while (parser.current_token_id < tokens.size) {
        Token token = current(&parser);

        switch (token) {
            case Token_Plus:
            case Token_Asterisk:
            case Token_Equal:
            case Token_Invalid: {
                fprintf(stderr, "Invalid token: '%d'\n", token);
                free(parser.nodes);
                return (UntypedAst) { NULL, NULL, NULL };
            } break;
            case Token_Identifier: {
                if (peek(&parser) == Token_Equal) {
                    node = assignment(&parser);
                } else {
                    node = identifier(&parser);
                }
            } break;
            case Token_Number: {
                node = expression(&parser);
            } break;
            case Token_Eof: {
                for (size_t i = 0; i < parser.stack_count; ++i) {
                    parser.views[parser.view_count++] = parser.stack[i];
                }
                parser.views[parser.view_count++] = 0;

                Node** statements = parser.views + (parser.view_count - parser.stack_count - 1);
                NodeBlock block = { NodeKind_Block, .nodes = statements };

                parser.stack_count = 0;
                parser.view_count += parser.stack_count + 1;

                node = add_node(&parser, (Node) { .block = block });

                return (UntypedAst) { parser.nodes, node, parser.views };
            }
        }

        if (node == 0)
            return (UntypedAst) { NULL, NULL, NULL };

        parser.stack[parser.stack_count++] = node;
    }

    fprintf(stderr, "Unexpected end of token stream\n");
    free(parser.nodes);
    return (UntypedAst) { NULL, NULL, NULL };
}
