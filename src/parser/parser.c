#include "parser.h"
#include "memory.h"
#include "error.h"


void parser_free(Parser* parser) {
    dealloc(parser->nodes);
    dealloc(parser->views);
    dealloc(parser->stack);
    token_array_free(parser->tokens);
}

Token current(const Parser* parser) {
    return parser->tokens.tokens[parser->token_index];
}

Token peek(const Parser* parser) {
    if (parser->token_index + 1 >= parser->tokens.size)
        return Token_Eof;
    return parser->tokens.tokens[parser->token_index + 1];
}

Token peek_back(const Parser* parser) {
    if (parser->token_index > 1)
        return parser->tokens.tokens[parser->token_index - 1];
    return Token_Eof;
}


TokenIndex advance(Parser* parser) {
    return parser->token_index++;
}

const char* repr_of_current(const Parser* parser) {
    return lexer_repr_of(parser->tokens, parser->token_index);
}

Node* add_node(Parser* parser, Node node) {
    NodeId id = parser->node_count++;
    parser->nodes[id] = node;
    return &parser->nodes[id];
}

NodeId reserve_node(Parser* parser) {
    NodeId id = parser->node_count++;
    return id;
}

Node* set_node(Parser* parser, NodeId id, Node node) {
    parser->nodes[id] = node;
    return &parser->nodes[id];
}

NodeId stack_snapshot(Parser* parser) {
    return parser->stack_count;
}

void stack_push(Parser* parser, Node* node) {
    parser->stack[parser->stack_count++] = node;
}

Node** stack_restore(Parser* parser, NodeId snapshot) {
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
