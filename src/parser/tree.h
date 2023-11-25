#pragma once

#include "types.h"
#include "node.h"
#include "logger.h"
#include "lexer/lexer.h"
#include "location.h"


typedef struct {
    const TokenArray tokens;

    Node*  nodes;
    Node*  start;
    Node** views;

    size_t block_count;
} GrammarTree;

void grammar_tree_free(GrammarTree ast);
GrammarTree parse(TokenArray tokens, Logger* logger);

static inline Location node_location(const GrammarTree* ast, Node* node) {
    return location_of(ast->tokens.source.data, ast->tokens.source_offsets[node->base.start]);
}

static inline NodeId node_id(const GrammarTree* ast, Node* node) {
    return (NodeId) (node - ast->nodes);
}
