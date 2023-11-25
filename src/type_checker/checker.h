#pragma once

#include "types.h"
#include "logger.h"
#include "parser/tree.h"


typedef u64 TypeId;
typedef struct Local {
    TypeId type;
    Node*  decl;
} Local;

typedef struct TypeInfo {
    const char* name;
    int size;
    int align;
    Node* decl;
} TypeInfo;

typedef struct Block {
    int     parent;
    i32     parent_count;
    Local*  locals;
    i64     count;
} Block;

typedef struct TypedAst {
    const GrammarTree tree;

    // Type checked info.
    Block*  block;
    i64     count;

    TypeId* types;
    TypeId  type_count;

    TypeInfo* type_info;
    TypeId type_info_count;
} TypedAst;

TypedAst type_check(GrammarTree ast, Logger* logger);

void typed_ast_free(TypedAst ast);