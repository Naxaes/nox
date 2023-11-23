#pragma once

#include "types.h"
#include "logger.h"
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
    const GrammarTree tree;

    // Type checked info.
    Block*  block;
    i64     count;
} TypedAst;

TypedAst type_check(GrammarTree ast, Logger* logger);

void typed_ast_free(TypedAst ast);
