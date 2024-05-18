#include <gtest/gtest.h>
#include "gmock/gmock.h"


extern "C" {
#include "type_checker/checker.h"
}

TEST(ParserTest, Simple) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("fun main() {}");

    TokenArray tokens = lexer_lex(STR("test"), source, &logger);
    GrammarTree ast = parse(tokens, &logger);

    Node_Module module = *(Node_Module*) ast.nodes;

    ASSERT_EQ(module.base.kind, NodeKind_Module);
    ASSERT_EQ(module.stmt_count, 1);

    Node_Stmt* stmt = module.stmts[0];
    ASSERT_EQ(stmt->kind, NodeKind_FunDecl);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.name, "main") == 0);
}

TEST(ParserTest, SimpleWithArgs) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("fun main(arg1: int, arg2: int) {}");

    TokenArray tokens = lexer_lex(STR("test"), source, &logger);
    GrammarTree ast = parse(tokens, &logger);

    Node_Module module = *(Node_Module*) ast.nodes;

    ASSERT_EQ(module.base.kind, NodeKind_Module);
    ASSERT_EQ(module.stmt_count, 1);

    Node_Stmt* stmt = module.stmts[0];
    ASSERT_EQ(stmt->kind, NodeKind_FunDecl);
    ASSERT_EQ(stmt->as.fun_decl.base.kind, NodeKind_FunDecl);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.name, "main") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.count, 2);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.params.nodes[0].name, "arg1") == 0);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.params.nodes[1].name, "arg2") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.nodes[0].type, (const char*) (size_t) (LiteralType_Integer));
    ASSERT_EQ(stmt->as.fun_decl.params.nodes[1].type, (const char*) (size_t) (LiteralType_Integer));
}



TEST(ParserTest, SimpleWithArgsAndReturnAndBody) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("fun main(arg1: int, arg2: int) int { return arg1 + arg2 }");

    TokenArray tokens = lexer_lex(STR("test"), source, &logger);
    GrammarTree ast = parse(tokens, &logger);

    Node_Module module = *(Node_Module*) ast.nodes;

    ASSERT_EQ(module.base.kind, NodeKind_Module);
    ASSERT_EQ(module.stmt_count, 1);

    Node_Stmt* stmt = module.stmts[0];
    ASSERT_EQ(stmt->kind, NodeKind_FunDecl);
    ASSERT_EQ(stmt->as.fun_decl.base.kind, NodeKind_FunDecl);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.name, "main") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.count, 2);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.params.nodes[0].name, "arg1") == 0);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.params.nodes[1].name, "arg2") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.nodes[0].type, (const char*) (size_t) (LiteralType_Integer));
    ASSERT_EQ(stmt->as.fun_decl.params.nodes[1].type, (const char*) (size_t) (LiteralType_Integer));
    ASSERT_EQ(stmt->as.fun_decl.return_type, (const char*) (size_t) (LiteralType_Integer));
    ASSERT_EQ(stmt->as.fun_decl.body.count, 1);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->kind, NodeKind_Return);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.return_stmt.expr->kind, NodeKind_Binary);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.return_stmt.expr->as.binary.op, BinaryOp_Add);
}

TEST(ParserTest, SimpleWithArgsAndReturnAndBodyAndVarDecl) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("fun main(arg1: int, arg2: int) int { a := arg1 + arg2 return a }");

    TokenArray tokens = lexer_lex(STR("test"), source, &logger);
    GrammarTree ast = parse(tokens, &logger);

    Node_Module module = *(Node_Module *) ast.nodes;

    ASSERT_EQ(module.base.kind, NodeKind_Module);
    ASSERT_EQ(module.stmt_count, 1);

    Node_Stmt* stmt = module.stmts[0];
    ASSERT_EQ(stmt->kind, NodeKind_FunDecl);
    ASSERT_EQ(stmt->as.fun_decl.base.kind, NodeKind_FunDecl);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.name, "main") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.count, 2);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.params.nodes[0].name, "arg1") == 0);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.params.nodes[1].name, "arg2") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.nodes[0].type, (const char *) (size_t) (LiteralType_Integer));
    ASSERT_EQ(stmt->as.fun_decl.params.nodes[1].type, (const char *) (size_t) (LiteralType_Integer));
    ASSERT_EQ(stmt->as.fun_decl.return_type, (const char *) (size_t) (LiteralType_Integer));
    ASSERT_EQ(stmt->as.fun_decl.body.count, 2);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->kind, NodeKind_VarDecl);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.base.kind, NodeKind_VarDecl);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.body.nodes[0]->as.var_decl.name, "a") == 0);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.type, (const char*)0);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.expr->kind, NodeKind_Binary);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.expr->as.binary.op, BinaryOp_Add);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[1]->kind, NodeKind_Return);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[1]->as.return_stmt.expr->kind, NodeKind_Identifier);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.body.nodes[1]->as.return_stmt.expr->as.identifier.name, "a") == 0);
}


TEST(ParserTest, MultipleFunctionDeclarations) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("fun main() int { return 1 * 2 + 3 } fun test(t: int) int { a := 1 + 2 * 3 return a / t }");

    TokenArray tokens = lexer_lex(STR("test"), source, &logger);
    GrammarTree ast = parse(tokens, &logger);

    Node_Module module = *(Node_Module *) ast.nodes;

    ASSERT_EQ(module.base.kind, NodeKind_Module);
    ASSERT_EQ(module.stmt_count, 2);

    Node_Stmt* stmt = module.stmts[0];
    ASSERT_EQ(stmt->kind, NodeKind_FunDecl);
    ASSERT_EQ(stmt->as.fun_decl.base.kind, NodeKind_FunDecl);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.name, "main") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.count, 0);
    ASSERT_EQ(stmt->as.fun_decl.return_type, (const char*)LiteralType_Integer);
    ASSERT_EQ(stmt->as.fun_decl.body.count, 1);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->kind, NodeKind_Return);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.return_stmt.expr->kind, NodeKind_Binary);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.return_stmt.expr->as.binary.op, BinaryOp_Add);

    stmt = module.stmts[1];
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.name, "test") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.count, 1);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.params.nodes[0].name, "t") == 0);
    ASSERT_EQ(stmt->as.fun_decl.params.nodes[0].type, (const char*)LiteralType_Integer);
    ASSERT_EQ(stmt->as.fun_decl.return_type, (const char*)LiteralType_Integer);
    ASSERT_EQ(stmt->as.fun_decl.body.count, 2);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->kind, NodeKind_VarDecl);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.base.kind, NodeKind_VarDecl);
    ASSERT_TRUE(strcmp(stmt->as.fun_decl.body.nodes[0]->as.var_decl.name, "a") == 0);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.type, (const char*)0);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.expr->kind, NodeKind_Binary);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[0]->as.var_decl.expr->as.binary.op, BinaryOp_Add);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[1]->kind, NodeKind_Return);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[1]->as.return_stmt.expr->kind, NodeKind_Binary);
    ASSERT_EQ(stmt->as.fun_decl.body.nodes[1]->as.return_stmt.expr->as.binary.op, BinaryOp_Div);
}


TEST(ParserTest, IfStmt) {
    Logger logger = logger_make_with_file("test", LOG_LEVEL_DEBUG, stderr);
    Str source = STR("fun main() int { if 1 == 2 { return 1 } else { return 2 } }");

    TokenArray tokens = lexer_lex(STR("test"), source, &logger);
    GrammarTree ast = parse(tokens, &logger);

    Node_Module module = *(Node_Module *) ast.nodes;

    ASSERT_EQ(module.base.kind, NodeKind_Module);
    ASSERT_EQ(module.stmt_count, 1);

    Node_Stmt* stmt = module.stmts[0];
    ASSERT_EQ(stmt->kind, NodeKind_FunDecl);

    Node_FunDecl fun_decl = stmt->as.fun_decl;
    ASSERT_EQ(fun_decl.kind, NodeKind_FunDecl);
    ASSERT_TRUE(strcmp(fun_decl.name, "main") == 0);
    ASSERT_EQ(fun_decl.params.count, 0);
    ASSERT_EQ(fun_decl.return_type, (const char*)LiteralType_Integer);

    ASSERT_EQ(fun_decl.body.count, 1);
    ASSERT_EQ(fun_decl.body.nodes[0]->kind, NodeKind_If);
    Node_If if_stmt = fun_decl.body.nodes[0]->as.if_stmt;
    ASSERT_EQ(if_stmt.condition->kind, NodeKind_Binary);
    Node_Cond* condition = if_stmt.condition;
    ASSERT_EQ(condition->as.binary.op, BinaryOp_Eq);

    Node_Stmt* then_stmt = if_stmt.then_stmt;
    ASSERT_EQ(then_stmt->kind, NodeKind_Block);
    Node_Block then_block = then_stmt->as.block;
    ASSERT_EQ(then_block.count, 1);
    ASSERT_EQ(then_block.nodes[0]->kind, NodeKind_Return);
    Node_Return return_stmt = then_block.nodes[0]->as.return_stmt;
    ASSERT_EQ(return_stmt.expr->kind, NodeKind_Literal);
    ASSERT_EQ(return_stmt.expr->as.literal.type, LiteralType_Integer);
    ASSERT_EQ(return_stmt.expr->as.literal.value.integer, 1);

    Node_Stmt* else_stmt = if_stmt.else_stmt;
    ASSERT_EQ(else_stmt->kind, NodeKind_Block);
    Node_Block else_block = else_stmt->as.block;
    ASSERT_EQ(else_block.count, 1);
    ASSERT_EQ(else_block.nodes[0]->kind, NodeKind_Return);
    return_stmt = else_block.nodes[0]->as.return_stmt;
    ASSERT_EQ(return_stmt.expr->kind, NodeKind_Literal);
    ASSERT_EQ(return_stmt.expr->as.literal.type, LiteralType_Integer);
    ASSERT_EQ(return_stmt.expr->as.literal.value.integer, 2);
}





