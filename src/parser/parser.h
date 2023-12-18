#pragma once

#include "types.h"
#include "node.h"
#include "logger.h"
#include "lexer/lexer.h"


typedef struct {
    Logger* logger;
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

    // NOTE: This is so we can differentiate between a brace initializer
    //       and the start of a block.
    //       Brace initializers are allowed if wrapped in parentheses.
    //       Example:
    //          if my_foo ==  Foo { a = 10 }  {}  // Not allowed, ambiguous.
    //          if my_foo == (Foo { a = 10 }) {}  // Allowed.
    int is_in_expression_where_body_follows;

    int struct_field_offset;
} Parser;

void parser_free(Parser* parser);

Token current(const Parser* parser);
Token peek(const Parser* parser);
Token peek_back(const Parser* parser);
TokenIndex advance(Parser* parser);
const char* repr_of_current(const Parser* parser);
Node* add_node(Parser* parser, Node node);
NodeId reserve_node(Parser* parser);
Node* set_node(Parser* parser, NodeId id, Node node);
NodeId stack_snapshot(Parser* parser);
void stack_push(Parser* parser, Node* node);
Node** stack_restore(Parser* parser, NodeId snapshot);
