#pragma once

#include "preamble.h"
#include "parser/parser.h"


typedef u64 TypeId;
typedef struct {
    TypeId type;
    Node*  decl;
} Local;

typedef struct {
    int     parent;
    i32     parent_count;
    Local*  locals;
    i64     count;
} Block;

typedef struct {
    // Extracted from the untyped ast.
    Node*  nodes;
    Node** views;
    Node*  start;

    // Type checked info.
    Block*  block;
} TypedAst;

TypedAst type_check(GrammarTree ast);

void typed_ast_free(TypedAst ast);

bool typed_ast_is_ok(TypedAst ast);
