#include "driver/driver.hpp"
#include "AST/AST.hpp"

#include <sstream>
#include <gtest/gtest.h>

TEST(ParserTest, SimpleAssignment) {
    std::stringstream input("x = 5;");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());

    auto& ast = driver.get_ast();
    ASSERT_NE(ast.root(), nullptr);

    auto scope = static_cast<const ast::ScopeNode*>(ast.root());
    const auto& stmts = scope->statements();
    ASSERT_EQ(stmts.size(), 1);

    EXPECT_EQ(stmts[0]->node_type(), ast::base_node_type::expr);
    auto exprNode = static_cast<const ast::ExprNode*>(stmts[0].get());
    ASSERT_NE(exprNode->expr(), nullptr);

    auto inner = exprNode->expr();
    EXPECT_EQ(inner->node_type(), ast::base_node_type::var_decl);
    auto varDecl = static_cast<const ast::VarDeclNode*>(inner);
    EXPECT_EQ(varDecl->name(), "x");
}

TEST(ParserTest, IfStatement) {
    std::stringstream input("if (x > 0) { y = 1; } else y = 0;");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());

    auto& ast = driver.get_ast();
    auto scope = static_cast<const ast::ScopeNode*>(ast.root());
    const auto& stmts = scope->statements();
    ASSERT_EQ(stmts.size(), 1);

    EXPECT_EQ(stmts[0]->node_type(), ast::base_node_type::if_node);
    auto ifNode = static_cast<const ast::IfNode*>(stmts[0].get());

    auto thenBranch = ifNode->then_branch();
    ASSERT_NE(thenBranch, nullptr);
    EXPECT_EQ(thenBranch->node_type(), ast::base_node_type::scope);

    auto thenScope = static_cast<const ast::ScopeNode*>(thenBranch);
    const auto& thenStmts = thenScope->statements();
    ASSERT_EQ(thenStmts.size(), 1);
    EXPECT_EQ(thenStmts[0]->node_type(), ast::base_node_type::expr);

    auto thenExpr = static_cast<const ast::ExprNode*>(thenStmts[0].get());
    ASSERT_NE(thenExpr->expr(), nullptr);
    EXPECT_EQ(thenExpr->expr()->node_type(), ast::base_node_type::var_decl);

    auto elseBranch = ifNode->else_branch();
    ASSERT_NE(elseBranch, nullptr);
    EXPECT_EQ(elseBranch->node_type(), ast::base_node_type::expr);

    auto elseExpr = static_cast<const ast::ExprNode*>(elseBranch);
    ASSERT_NE(elseExpr->expr(), nullptr);
    EXPECT_EQ(elseExpr->expr()->node_type(), ast::base_node_type::var_decl);
}

TEST(ParserTest, WhileLoop) {
    std::stringstream input("while (x < 10) { x = x + 1; }");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());

    auto& ast = driver.get_ast();
    const auto& stmts = static_cast<const ast::ScopeNode*>(ast.root())->statements();
    EXPECT_EQ(stmts.size(), 1);

    EXPECT_EQ(stmts[0]->node_type(), ast::base_node_type::while_node);
    auto while_node = static_cast<ast::WhileNode*>(stmts[0].get());
    EXPECT_EQ(while_node->condition()->node_type(), ast::base_node_type::bin_logic_op);
    EXPECT_EQ(while_node->body()->node_type(), ast::base_node_type::scope);
}

TEST(ParserTest, PrintStatement) {
    std::stringstream input("print x;");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());

    auto& ast = driver.get_ast();
    const auto& stmts = static_cast<const ast::ScopeNode*>(ast.root())->statements();
    EXPECT_EQ(stmts.size(), 1);

    EXPECT_EQ(stmts[0]->node_type(), ast::base_node_type::print);
    auto print_node = static_cast<ast::PrintNode*>(stmts[0].get());
    EXPECT_EQ(print_node->expr()->node_type(), ast::base_node_type::var);
}

TEST(ParserTest, InvalidSyntaxMissingSemicolon) {
    std::stringstream input("x = 5");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());
    EXPECT_TRUE(driver.has_errors());
}

TEST(ParserTest, EmptyConditionError) {
    std::stringstream input("if () { }");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());
    EXPECT_TRUE(driver.has_errors());
}

TEST(ParserTest, EmptyInput) {
    std::stringstream input("");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());
    auto& ast = driver.get_ast();
    EXPECT_EQ(static_cast<const ast::ScopeNode*>(ast.root())->statements().size(), 0);
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}