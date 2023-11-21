#pragma once

#include "types.h"
#include "node.h"
#include "lexer/lexer.h"


typedef struct {
    const TokenArray tokens;

    Node*  nodes;
    Node*  start;
    Node** views;

    size_t block_count;
} GrammarTree;

GrammarTree parse(const TokenArray tokens);

void grammar_tree_free(GrammarTree ast);
