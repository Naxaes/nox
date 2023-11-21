#pragma once

#include "types.h"
#include "node.h"
#include "logger.h"
#include "lexer/lexer.h"


typedef struct {
    const TokenArray tokens;

    Node*  nodes;
    Node*  start;
    Node** views;

    size_t block_count;
} GrammarTree;

GrammarTree parse(TokenArray tokens, Logger* logger);

void grammar_tree_free(GrammarTree ast);
