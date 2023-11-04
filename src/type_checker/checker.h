#pragma once

#include "types.h"
#include "parser/parser.h"

typedef struct {
    Node* nodes;
} TypedAst;

TypedAst type_check(UntypedAst ast);

