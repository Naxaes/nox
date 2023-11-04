#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

typedef u32 NodeId;


typedef struct {
    TokenArray tokens;
    size_t     current_token;
    Node*      nodes;
    NodeId     node_count;
} Parser;

Token current(Parser* parser) {
    return parser->tokens.tokens[parser->current_token];
}

TokenId advance(Parser* parser) {
    return parser->current_token++;
}

NodeId add_node(Parser* parser, Node node) {
    NodeId id = parser->node_count++;
    parser->nodes[id] = node;
    return id;
}


NodeId parse_literal(Parser* parser) {
    Token token = current(parser);
    switch (token) {
        case Token_Literal: {
            Str repr = lexer_repr_of(parser->tokens, parser->current_token);
            i64 value = strtoll(repr.data, NULL, 10);
            advance(parser);

            NodeLiteral literal = { NodeKind_Literal, .value.integer = value };
            return add_node(parser, (Node) { .literal = literal });
        } break;
        default: {
            fprintf(stderr, "Invalid token literal: '%d'\n", token);
            return 0;
        } break;
    }
}


UntypedAst parse(TokenArray tokens) {
    Parser parser = {
        .tokens = tokens,
        .current_token = 0,
        .nodes = (Node*) malloc(1024),
        .node_count = 0,
    };

    while (parser.current_token < tokens.size) {
        Token token = current(&parser);
        switch (token) {
            case Token_Invalid: {
                fprintf(stderr, "Unknown token: '%d'\n", token);
                free(parser.nodes);
                return (UntypedAst) { NULL };
            } break;
            case Token_Literal: {
                parse_literal(&parser);
            } break;
            case Token_Eof: {
                return (UntypedAst) { parser.nodes };
            }
        }
    }

    fprintf(stderr, "Unexpected end of token stream\n");
    free(parser.nodes);
    return (UntypedAst) { NULL };
}
