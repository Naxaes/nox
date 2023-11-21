

#include "parser.h"


struct Visitor;

#define X(upper, lower, flags, body) typedef void* (*Visit##upper##Fn)(struct Visitor* visitor, Node##upper* node);
    ALL_NODES(X)
#undef X
typedef struct Visitor {
#define X(upper, lower, flags, body) void* (*visit_##lower)(struct Visitor* visitor, Node##upper* node);
    ALL_NODES(X)
#undef X
} Visitor;

void* visit(void* visitor, Node* node);

void* walk(Visitor* visitor, Node* node);
void* walk_view(Visitor* visitor, Node** nodes);
void* walk_literal(Visitor* visitor, NodeLiteral* node);
void* walk_identifier(Visitor* visitor, NodeIdentifier* node);
void* walk_binary(Visitor* visitor, NodeBinary* node);
void* walk_call(Visitor* visitor, NodeCall* node);
void* walk_type(Visitor* visitor, NodeType* node);
void* walk_assign(Visitor* visitor, NodeAssign* node);
void* walk_var_decl(Visitor* visitor, NodeVarDecl* node);
void* walk_block(Visitor* visitor, NodeBlock* node);
void* walk_fun_param(Visitor* visitor, NodeFunParam* node);
void* walk_fun_decl(Visitor* visitor, NodeFunDecl* node);
void* walk_return_stmt(Visitor* visitor, NodeReturn* node);
void* walk_if_stmt(Visitor* visitor, NodeIf* node);
void* walk_while_stmt(Visitor* visitor, NodeWhile* node);

