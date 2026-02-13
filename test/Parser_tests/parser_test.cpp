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
    EXPECT_EQ(ast.root()->node_type(), ast::base_node_type::scope);

    const auto& stmts = static_cast<const ast::ScopeNode*>(ast.root())->statements();
    EXPECT_EQ(stmts.size(), 1);

    EXPECT_EQ(stmts[0]->node_type(), ast::base_node_type::var_decl);
    auto decl = static_cast<ast::VarDeclNode*>(stmts[0].get());
    EXPECT_EQ(decl->name(), "x");
    auto init = decl->init_expr();
    ASSERT_NE(init, nullptr);
    EXPECT_EQ(init->node_type(), ast::base_node_type::value);
    EXPECT_EQ(static_cast<ast::ValueNode*>(init)->value(), 5);
}

TEST(ParserTest, IfStatement) {
    std::stringstream input("if (x > 0) { y = 1; } else y = 0;");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_TRUE(driver.parse());

    auto& ast = driver.get_ast();
    const auto& stmts = static_cast<const ast::ScopeNode*>(ast.root())->statements();
    EXPECT_EQ(stmts.size(), 1);

    EXPECT_EQ(stmts[0]->node_type(), ast::base_node_type::if_node);
    auto if_node = static_cast<ast::IfNode*>(stmts[0].get());
    EXPECT_EQ(if_node->condition()->node_type(), ast::base_node_type::bin_logic_op);
    EXPECT_EQ(if_node->then_branch()->node_type(), ast::base_node_type::scope);
    EXPECT_EQ(if_node->else_branch()->node_type(), ast::base_node_type::var_decl);
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

    EXPECT_FALSE(driver.parse());  // Should fail
}

TEST(ParserTest, EmptyConditionError) {
    std::stringstream input("if () { }");
    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer);

    EXPECT_FALSE(driver.parse());  // Fails
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