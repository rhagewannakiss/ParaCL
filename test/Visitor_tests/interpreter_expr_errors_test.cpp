#include "AST/AST.hpp"
#include "Visitors/Interpreter.hpp"
#include "gtest/gtest.h"

TEST(InterpreterExprErrorTest, ExprNodeMissingExprTest) {
    ast::Interpreter interpreter;
    ast::ExprNode node;
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterExprErrorTest, UnOpNodeMissingOperandTest) {
    ast::Interpreter interpreter;
    ast::UnOpNode node(ast::unop_node_type::neg);
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterExprErrorTest, BinArithOpNodeMissingOperandTest) {
    ast::Interpreter interpreter;

    {
        ast::BinArithOpNode node(
            ast::bin_arith_op_type::add,
            std::make_unique<ast::ValueNode>(1),
            nullptr);
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::BinArithOpNode node(
            ast::bin_arith_op_type::add,
            nullptr,
            std::make_unique<ast::ValueNode>(1));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(InterpreterExprErrorTest, BinLogicOpNodeMissingOperandTest) {
    ast::Interpreter interpreter;

    {
        ast::BinLogicOpNode node(
            ast::bin_logic_op_type::equal,
            std::make_unique<ast::ValueNode>(1),
            nullptr);
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::BinLogicOpNode node(
            ast::bin_logic_op_type::equal,
            nullptr,
            std::make_unique<ast::ValueNode>(1));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(InterpreterExprErrorTest, ForNodeCannotBeUsedAsPrintExpression) {
    ast::Interpreter interpreter;
    ast::PrintNode node(std::make_unique<ast::ForNode>(
        nullptr,
        std::make_unique<ast::ValueNode>(0),
        nullptr,
        std::make_unique<ast::ScopeNode>()));

    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}
