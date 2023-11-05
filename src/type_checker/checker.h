#pragma once

#include "types.h"
#include "parser/parser.h"


typedef u32 TypeId;

typedef struct {
    Node*   nodes;
    TypeId* types;
    NodeId  start;
} TypedAst;

TypedAst type_check(UntypedAst ast);

