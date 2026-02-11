#include "AST/AST.hpp"
#include "Visitors/Interpreter.hpp"
#include "gtest/gtest.h"
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
} // namespace

TEST(InterpreterControlFlowTest, IfNodeTrueBranchTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(0)));
    root.add_statement(std::make_unique<ast::IfNode>(
        std::make_unique<ast::ValueNode>(1),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(10)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(20))));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "10\n");
}

TEST(InterpreterControlFlowTest, IfNodeFalseBranchTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(0)));
    root.add_statement(std::make_unique<ast::IfNode>(
        std::make_unique<ast::ValueNode>(0),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(10)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(20))));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "20\n");
}

TEST(InterpreterControlFlowTest, IfNodeWithoutElseTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(7)));
    root.add_statement(std::make_unique<ast::IfNode>(
        std::make_unique<ast::ValueNode>(0),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(10)),
        nullptr));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "7\n");
}

TEST(InterpreterControlFlowTest, IfNodeMissingThenWhenTrueThrows) {
    ast::Interpreter interpreter;
    ast::IfNode node(std::make_unique<ast::ValueNode>(1), nullptr, nullptr);
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterControlFlowTest, WhileNodeZeroIterationsTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(0)));
    root.add_statement(std::make_unique<ast::WhileNode>(
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)))));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "0\n");
}

TEST(InterpreterControlFlowTest, WhileNodeMultipleIterationsTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(0)));
    root.add_statement(std::make_unique<ast::WhileNode>(
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(3)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)))));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "3\n");
}

TEST(InterpreterControlFlowTest, WhileNodeMissingConditionThrows) {
    ast::Interpreter interpreter;
    ast::WhileNode node(nullptr, std::make_unique<ast::ExprNode>(
        std::make_unique<ast::ValueNode>(1)));
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterControlFlowTest, WhileNodeMissingBodyThrows) {
    ast::Interpreter interpreter;
    ast::WhileNode node(std::make_unique<ast::ValueNode>(1), nullptr);
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterControlFlowTest, ScopeNodeShadowingTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(1)));

    auto inner = std::make_unique<ast::ScopeNode>();
    inner->add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(2)));
    inner->add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));
    root.add_statement(std::move(inner));

    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "2\n1\n");
}
