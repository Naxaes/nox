#pragma once

#include "types.h"
#include "parser/parser.h"


typedef u32 TypeId;
typedef struct {
    TypeId type;
    Node*  decl;
} Local;

typedef struct {
    size_t  parent;
    Local*  locals;
    size_t  count;
} Block;

typedef struct {
    // Extracted from the untyped ast.
    Node*  nodes;
    Node** views;
    Node*  start;

    // Type checked info.
    Block*  block;
} TypedAst;

TypedAst type_check(UntypedAst ast);

void typed_ast_free(TypedAst ast);
