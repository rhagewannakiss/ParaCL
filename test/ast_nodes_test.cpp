#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include "../include/AST.hpp"

TEST(BaseSmokeTest, ValueNodeTest) {
    ast::ValueNode v(42);
    EXPECT_EQ(v.value(), 42);
    EXPECT_EQ(v.node_type(), ast::base_node_type::value);
}

TEST(BaseSmokeTest, VarNodeTest) {
    ast::VarNode var("a");
    EXPECT_EQ(var.name(), "a");
    EXPECT_EQ(var.node_type(), ast::base_node_type::var);
}

TEST(BaseSmokeTest, UnOpNodeTest) {
    auto op = std::make_unique<ast::VarNode>("a");
    ast::UnOpNode uo(ast::unop_node_type::neg, std::move(op));

    EXPECT_EQ(uo.node_type(), ast::base_node_type::unop);
    EXPECT_EQ(uo.op(), ast::unop_node_type::neg);
    ASSERT_NE(uo.operand(), nullptr);
    EXPECT_EQ(uo.operand()->node_type(), ast::base_node_type::var);
}

TEST(BaseSmokeTest, PrintNodeTest) {
    auto op = std::make_unique<ast::ValueNode>(69); 
    ast::PrintNode p(std::move(op));

    EXPECT_EQ(p.node_type(), ast::base_node_type::print);
    EXPECT_NE(p.expr(), nullptr);
}

TEST(PrintNodeUnitTest, CheckNodeTypesInExpr) {
    auto un_var = std::make_unique<ast::VarNode>("var");
    auto op = std::make_unique<ast::InputNode>(std::move(un_var));

    EXPECT_THROW(ast::PrintNode p(std::move(op)), std::logic_error);
}

TEST(BaseSmokeTest, ExprNodeTest) {
    auto v = std::make_unique<ast::ValueNode>(69);
    ast::ExprNode e(std::move(v));

    EXPECT_EQ(e.node_type(), ast::base_node_type::expr);
    EXPECT_NE(e.expr(), nullptr);
}

TEST(BaseSmokeTest, InputNodeTest) {
    auto var = std::make_unique<ast::VarNode>("var");
    ast::InputNode i(std::move(var));

    EXPECT_EQ(i.node_type(), ast::base_node_type::input);
    EXPECT_EQ(i.lhs()->node_type(), ast::base_node_type::var);
}

TEST(InputNodeUnitTest, CheckAvailableNodesForInput) {
    auto var = std::make_unique<ast::VarNode>("var");

    EXPECT_THROW(ast::InputNode i(std::move(var)), std::logic_error);

    //TODO: add more node types
}

TEST(BaseSmokeTest, AssignNodeTest) {
    auto lhs = std::make_unique<ast::VarNode>("var");
    auto rhs = std::make_unique<ast::ValueNode>(69);
    ast::AssignNode a(std::move(lhs), std::move(rhs));
    
    EXPECT_EQ(a.node_type(), ast::base_node_type::assign);
    EXPECT_NE(a.lhs(), nullptr);
    EXPECT_NE(a.rhs(), nullptr);
}

TEST(BaseSmokeTest, IfNodeTest) {
    auto v1 = std::make_unique<ast::ValueNode>(69);
    auto v2 = std::make_unique<ast::ValueNode>(96);
    auto c = std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::greater,
            std::move(v1),
            std::move(v2)
            );
}
