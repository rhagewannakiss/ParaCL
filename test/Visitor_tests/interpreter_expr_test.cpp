#include "AST/AST.hpp"
#include "Visitors/Interpreter.hpp"
#include "gtest/gtest.h"
#include <limits>
#include <sstream>

namespace {
std::string RunAndCapture(ast::BaseNode& node) {
    ast::Interpreter interpreter;
    std::ostringstream out;
    std::streambuf* old = std::cout.rdbuf(out.rdbuf());
    try {
        node.accept(interpreter);
    } catch (...) {
        std::cout.rdbuf(old);
        throw;
    }
    std::cout.rdbuf(old);
    return out.str();
}

std::string RunValueNodePrintCase(int64_t value) {
    ast::PrintNode node(std::make_unique<ast::ValueNode>(value));
    return RunAndCapture(node);
}

void AddPrintedBinaryExpr(
    ast::ScopeNode& scope,
    ast::bin_arith_op_type op,
    int64_t lhs,
    int64_t rhs)
{
    scope.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::BinArithOpNode>(
            op,
            std::make_unique<ast::ValueNode>(lhs),
            std::make_unique<ast::ValueNode>(rhs))));
}

void AddPrintedBinaryExpr(
    ast::ScopeNode& scope,
    ast::bin_logic_op_type op,
    int64_t lhs,
    int64_t rhs)
{
    scope.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::BinLogicOpNode>(
            op,
            std::make_unique<ast::ValueNode>(lhs),
            std::make_unique<ast::ValueNode>(rhs))));
}
} // namespace

TEST(InterpreterExprTest, ValueNodeTest) {
    EXPECT_EQ(RunValueNodePrintCase(0), "0\n");
    EXPECT_EQ(RunValueNodePrintCase(-1), "-1\n");
    
    EXPECT_EQ(RunValueNodePrintCase(
                std::numeric_limits<int64_t>::max()), 
                "9223372036854775807\n");
    EXPECT_EQ(RunValueNodePrintCase(
                std::numeric_limits<int64_t>::min()), 
                "-9223372036854775808\n");
}

TEST(InterpreterExprTest, VarNodeTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(42))
    );
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x"))
    );

    EXPECT_EQ(RunAndCapture(root), "42\n");
}

TEST(InterpreterExprTest, UnOpNodeTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::UnOpNode>(
            ast::unop_node_type::neg,
            std::make_unique<ast::ValueNode>(5))));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::UnOpNode>(
            ast::unop_node_type::pos,
            std::make_unique<ast::ValueNode>(7))));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::UnOpNode>(
            ast::unop_node_type::logical_not,
            std::make_unique<ast::ValueNode>(0))));

    EXPECT_EQ(RunAndCapture(root), "-5\n7\n1\n");
}

TEST(InterpreterExprTest, BinArithOpNodeTest) {
    ast::ScopeNode root;
    AddPrintedBinaryExpr(root, ast::bin_arith_op_type::add, 8, 3);
    AddPrintedBinaryExpr(root, ast::bin_arith_op_type::sub, 8, 3);
    AddPrintedBinaryExpr(root, ast::bin_arith_op_type::mul, 8, 3);
    AddPrintedBinaryExpr(root, ast::bin_arith_op_type::div, 8, 3);
    AddPrintedBinaryExpr(root, ast::bin_arith_op_type::mod, 8, 3);

    EXPECT_EQ(RunAndCapture(root), "11\n5\n24\n2\n2\n");
}

TEST(InterpreterExprTest, BinLogicOpNodeTest) {
    ast::ScopeNode root;
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::greater, 8, 3);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::less, 8, 3);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::greater_equal, 8, 8);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::less_equal, 3, 8);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::equal, 5, 5);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::not_equal, 5, 6);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::logical_and, 1, 0);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::logical_or, 0, 7);
    AddPrintedBinaryExpr(root, ast::bin_logic_op_type::bitwise_xor, 6, 3);

    EXPECT_EQ(RunAndCapture(root), "1\n0\n1\n1\n1\n1\n0\n1\n5\n");
}

TEST(InterpreterExprTest, ExprNodeTest) {
    ast::PrintNode valid_node(std::make_unique<ast::ExprNode>(
        std::make_unique<ast::ValueNode>(123)));
    EXPECT_EQ(RunAndCapture(valid_node), "123\n");
}

TEST(InterpreterExprTest, ForNodeUsesExpressionNodesInCondAndStepTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(0)));

    root.add_statement(std::make_unique<ast::ForNode>(
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(0)),
            std::make_unique<ast::ValueNode>(3)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1))),
        std::make_unique<ast::ScopeNode>()));

    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "3\n");
}
