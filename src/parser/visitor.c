

#include "visitor.h"


void* visit(Visitor* visitor, Node* node) {
    switch (node->kind) {
        case NodeKind_Literal:
            if (visitor->visit_literal)
                return visitor->visit_literal(visitor, (NodeLiteral*) node);
            else
                return walk_literal(visitor, (NodeLiteral*) node);
        case NodeKind_Identifier:
            if (visitor->visit_identifier)
                return visitor->visit_identifier(visitor, (NodeIdentifier*) node);
            else
                return walk_identifier(visitor, (NodeIdentifier*) node);
        case NodeKind_Binary:
            if (visitor->visit_binary)
                return visitor->visit_binary(visitor, (NodeBinary*) node);
            else
                return walk_binary(visitor, (NodeBinary*) node);
        case NodeKind_Call:
            if (visitor->visit_call)
                return visitor->visit_call(visitor, (NodeCall*) node);
            else
                return walk_call(visitor, (NodeCall*) node);
        case NodeKind_Assign:
            if (visitor->visit_assign)
                return visitor->visit_assign(visitor, (NodeAssign*) node);
            else
                return walk_assign(visitor, (NodeAssign*) node);
        case NodeKind_VarDecl:
            if (visitor->visit_var_decl)
                return visitor->visit_var_decl(visitor, (NodeVarDecl*) node);
            else
                return walk_var_decl(visitor, (NodeVarDecl*) node);
        case NodeKind_Block:
            if (visitor->visit_block)
                return visitor->visit_block(visitor, (NodeBlock*) node);
            else
                return walk_block(visitor, (NodeBlock*) node);
        case NodeKind_FunParam:
            if (visitor->visit_fun_param)
                return visitor->visit_fun_param(visitor, (NodeFunParam*) node);
            else
                return walk_fun_param(visitor, (NodeFunParam*) node);
        case NodeKind_FunDecl:
            if (visitor->visit_fun_decl)
                return visitor->visit_fun_decl(visitor, (NodeFunDecl*) node);
            else
                return walk_fun_decl(visitor, (NodeFunDecl*) node);
        case NodeKind_If:
            if (visitor->visit_if_stmt)
                return visitor->visit_if_stmt(visitor, (NodeIf*) node);
            else
                return walk_if_stmt(visitor, (NodeIf*) node);
        case NodeKind_While:
            if (visitor->visit_while_stmt)
                return visitor->visit_while_stmt(visitor, (NodeWhile*) node);
            else
                return walk_while_stmt(visitor, (NodeWhile*) node);
    }
}


void* walk(Visitor* visitor, Node* node) {
    switch (node->kind) {
        case NodeKind_Literal:
            return NULL;
        case NodeKind_Identifier:
            return NULL;
        case NodeKind_Binary:      
            visit(visitor, node->binary.left);
            visit(visitor, node->binary.right);
            return NULL;
        case NodeKind_Call:
            walk_view(visitor, (Node**)node->call.args);
            return NULL;
        case NodeKind_Assign:
            visit(visitor, node->assign.expression);
            return NULL;
        case NodeKind_VarDecl:
            visit(visitor, node->var_decl.expression);
            return NULL;
        case NodeKind_Block:       
            walk_view(visitor, node->block.nodes);
            return NULL;
        case NodeKind_FunParam:    
            visit(visitor, node->fun_param.expression);
            return NULL;
        case NodeKind_FunDecl:
            walk_view(visitor, (Node**)node->fun_decl.params);
            visit(visitor, (Node*) node->fun_decl.block);
            return NULL;
        case NodeKind_If:
            visit(visitor, node->if_stmt.condition);
            visit(visitor, (Node*)node->if_stmt.then_block);
            if (node->if_stmt.else_block)
                visit(visitor, (Node*)node->if_stmt.else_block);
            return NULL;
        case NodeKind_While:
            visit(visitor, node->while_stmt.condition);
            visit(visitor, (Node*)node->while_stmt.then_block);
            if (node->while_stmt.else_block)
                visit(visitor, (Node*)node->while_stmt.else_block);
            return NULL;
    }
}

void* walk_view(Visitor* visitor, Node** nodes) {
    if (nodes == NULL)
        return NULL;

    Node* node = *nodes;
    while (node != NULL) {
        visit(visitor, node);
        node++;
    }
    return NULL;
}

void* walk_literal(Visitor* visitor, NodeLiteral* node) {
    (void) visitor;
    (void) node;
    return NULL;
}

void* walk_identifier(Visitor* visitor, NodeIdentifier* node) {
    (void) visitor;
    (void) node;
    return NULL;
}

void* walk_binary(Visitor* visitor, NodeBinary* node) {
    visit(visitor, node->left);
    visit(visitor, node->right);
    return NULL;
}

void* walk_call(Visitor* visitor, NodeCall* node) {
    walk_view(visitor, (Node*)node->args);
    return NULL;
}

void* walk_assign(Visitor* visitor, NodeAssign* node) {
    visit(visitor, node->expression);
    return NULL;
}

void* walk_var_decl(Visitor* visitor, NodeVarDecl* node) {
    visit(visitor, node->expression);
    return NULL;
}

void* walk_block(Visitor* visitor, NodeBlock* node) {
    walk_view(visitor, (Node**)node->nodes);
    return NULL;
}

void* walk_fun_param(Visitor* visitor, NodeFunParam* node) {
    visit(visitor, node->expression);
    return NULL;
}

void* walk_fun_decl(Visitor* visitor, NodeFunDecl* node) {
    walk_view(visitor, (Node**)node->params);
    visit(visitor, (Node*)node->block);
    return NULL;
}

void* walk_if_stmt(Visitor* visitor, NodeIf* node) {
    visit(visitor, node->condition);
    visit(visitor, (Node*)node->then_block);
    if (node->else_block)
        visit(visitor, (Node*)node->else_block);
    return NULL;
}

void* walk_while_stmt(Visitor* visitor, NodeWhile* node) {
    visit(visitor, node->condition);
    visit(visitor, (Node*)node->then_block);
    if (node->else_block)
        visit(visitor, (Node*)node->else_block);
    return NULL;
}


