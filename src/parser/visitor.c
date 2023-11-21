

#include "visitor.h"


void* visit(void* visitor, Node* node) {
    Visitor* impl = (Visitor*) visitor;
    switch (node->kind) {
        case NodeKind_Literal:
            return impl->visit_literal(impl, (NodeLiteral*) node);
        case NodeKind_Identifier:
            return impl->visit_identifier(impl, (NodeIdentifier*) node);
        case NodeKind_Binary:
            return impl->visit_binary(impl, (NodeBinary*) node);
        case NodeKind_Call:
            return impl->visit_call(impl, (NodeCall*) node);
        case NodeKind_Type:
            return impl->visit_type(impl, (NodeType*) node);
        case NodeKind_Assign:
            return impl->visit_assign(impl, (NodeAssign*) node);
        case NodeKind_VarDecl:
            return impl->visit_var_decl(impl, (NodeVarDecl*) node);
        case NodeKind_Block:
            return impl->visit_block(impl, (NodeBlock*) node);
        case NodeKind_FunParam:
            return impl->visit_fun_param(impl, (NodeFunParam*) node);
        case NodeKind_FunBody:
            return impl->visit_fun_body(impl, (NodeFunBody*) node);
        case NodeKind_FunDecl:
            return impl->visit_fun_decl(impl, (NodeFunDecl*) node);
        case NodeKind_Return:
            return impl->visit_return_stmt(impl, (NodeReturn*) node);
        case NodeKind_If:
            return impl->visit_if_stmt(impl, (NodeIf*) node);
        case NodeKind_While:
            return impl->visit_while_stmt(impl, (NodeWhile*) node);
        case NodeKind_Module:
            return impl->visit_module(impl, (NodeModule*) node);
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
        case NodeKind_Type:
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
        case NodeKind_FunBody:
            walk_view(visitor, node->block.nodes);
            return NULL;
        case NodeKind_FunDecl:
            walk_view(visitor, (Node**)node->fun_decl.params);
            visit(visitor, (Node*) node->fun_decl.body);
            return NULL;
        case NodeKind_Return:
            visit(visitor, node->return_stmt.expression);
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
        case NodeKind_Module:
            walk_view(visitor, node->module.decls);
            walk_view(visitor, node->module.stmts);
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
    walk_view(visitor, (Node**)node->args);
    return NULL;
}

void* walk_type(Visitor* visitor, NodeType* node) {
    (void) visitor;
    (void) node;
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

void* walk_fun_body(Visitor* visitor, NodeFunBody* node) {
    walk_view(visitor, (Node**)node->nodes);
    return NULL;
}

void* walk_fun_decl(Visitor* visitor, NodeFunDecl* node) {
    walk_view(visitor, (Node**)node->params);
    visit(visitor, (Node*)node->body);
    return NULL;
}

void* walk_return_stmt(Visitor* visitor, NodeReturn* node) {
    visit(visitor, (Node*)node->expression);
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


