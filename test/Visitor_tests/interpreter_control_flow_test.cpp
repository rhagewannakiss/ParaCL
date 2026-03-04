#include "AST/AST.hpp"
#include "Visitors/Interpreter.hpp"
#include "gtest/gtest.h"
#include <sstream>

namespace {
std::string RunAndCapture(ast::BaseNode& node)
{
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

class ScopedCinInput
{
public:
    explicit ScopedCinInput(const std::string& input)
      : in_(input)
      , old_(std::cin.rdbuf(in_.rdbuf()))
    {
    }
    ~ScopedCinInput()
    {
        std::cin.rdbuf(old_);
    }

    ScopedCinInput(const ScopedCinInput&) = delete;
    ScopedCinInput& operator=(const ScopedCinInput&) = delete;

private:
    std::istringstream in_;
    std::streambuf* old_;
};
} // namespace

TEST(InterpreterControlFlowTest, IfNodeTrueBranchTest)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));
    root.add_statement(std::make_unique<ast::IfNode>(
        std::make_unique<ast::ValueNode>(1),
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::ValueNode>(10)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(20))));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "10\n");
}

TEST(InterpreterControlFlowTest, IfNodeFalseBranchTest)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));
    root.add_statement(std::make_unique<ast::IfNode>(
        std::make_unique<ast::ValueNode>(0),
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::ValueNode>(10)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(20))));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "20\n");
}

TEST(InterpreterControlFlowTest, IfNodeWithoutElseTest)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(7)));
    root.add_statement(std::make_unique<ast::IfNode>(
        std::make_unique<ast::ValueNode>(0),
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::ValueNode>(10)),
        nullptr));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "7\n");
}

TEST(InterpreterControlFlowTest, IfNodeMissingThenWhenTrueIsNoOp)
{
    ast::Interpreter interpreter;
    ast::IfNode node(std::make_unique<ast::ValueNode>(1), nullptr, nullptr);
    EXPECT_NO_THROW(node.accept(interpreter));
}

TEST(InterpreterControlFlowTest, WhileNodeZeroIterationsTest)
{
    ast::ScopeNode root;
    auto while_body = std::make_unique<ast::ScopeNode>();
    while_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::add,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1))));

    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));
    auto while_node = std::make_unique<ast::WhileNode>(nullptr, nullptr);
    while_node->set_condition(std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::less,
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::ValueNode>(0)));
    while_node->set_body(std::move(while_body));
    root.add_statement(std::move(while_node));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "0\n");
}

TEST(InterpreterControlFlowTest, WhileNodeMultipleIterationsTest)
{
    ast::ScopeNode root;
    auto while_body = std::make_unique<ast::ScopeNode>();
    while_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::add,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1))));

    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));
    auto while_node = std::make_unique<ast::WhileNode>(nullptr, nullptr);
    while_node->set_condition(std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::less,
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::ValueNode>(3)));
    while_node->set_body(std::move(while_body));
    root.add_statement(std::move(while_node));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "3\n");
}

TEST(InterpreterControlFlowTest,
     WhileConditionInputNodeReadsFreshValueEachCheck)
{
    ScopedCinInput input("3\n1\n");

    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));

    auto while_body = std::make_unique<ast::ScopeNode>();
    while_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::add,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1))));

    root.add_statement(std::make_unique<ast::WhileNode>(
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::InputNode>()),
        std::move(while_body)));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "1\n");
}

TEST(InterpreterControlFlowTest, WhileConditionAssignTracksVariableValue)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "y", std::make_unique<ast::ValueNode>(6)));

    auto while_body = std::make_unique<ast::ScopeNode>();
    while_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1))));
    while_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("y"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("y"),
            std::make_unique<ast::ValueNode>(2))));

    root.add_statement(std::make_unique<ast::WhileNode>(
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::VarNode>("y")),
        std::move(while_body)));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("y")));

    EXPECT_EQ(RunAndCapture(root), "-6\n");
}

TEST(InterpreterControlFlowTest, WhileConditionVarDeclTracksVariableValue)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "y", std::make_unique<ast::ValueNode>(6)));

    auto while_body = std::make_unique<ast::ScopeNode>();
    while_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1))));
    while_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("y"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("y"),
            std::make_unique<ast::ValueNode>(2))));

    root.add_statement(std::make_unique<ast::WhileNode>(
        std::make_unique<ast::VarDeclNode>("x",
                                           std::make_unique<ast::VarNode>("y")),
        std::move(while_body)));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("y")));

    EXPECT_EQ(RunAndCapture(root), "-6\n");
}

TEST(InterpreterControlFlowTest, WhileNodeMissingConditionThrows)
{
    ast::Interpreter interpreter;
    ast::WhileNode node(
        nullptr,
        std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1)));
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterControlFlowTest, WhileNodeMissingBodyWithFalseConditionIsNoOp)
{
    ast::Interpreter interpreter;
    ast::WhileNode node(std::make_unique<ast::ValueNode>(0), nullptr);
    EXPECT_NO_THROW(node.accept(interpreter));
}

TEST(InterpreterControlFlowTest, ForNodeMultipleIterationsTest)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));

    auto for_body = std::make_unique<ast::ScopeNode>();

    auto for_node = std::make_unique<ast::ForNode>(
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(3)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1))),
        std::move(for_body));

    root.add_statement(std::move(for_node));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "3\n");
}

TEST(InterpreterControlFlowTest, ForNodeZeroIterationsTest)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));

    auto for_node = std::make_unique<ast::ForNode>(
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1))),
        std::make_unique<ast::ScopeNode>());

    root.add_statement(std::move(for_node));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "0\n");
}

TEST(InterpreterControlFlowTest, ForConditionInputNodeReadsFreshValueEachCheck)
{
    ScopedCinInput input("4\n1\n");

    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(0)));

    root.add_statement(std::make_unique<ast::ForNode>(
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::InputNode>()),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1))),
        std::make_unique<ast::ScopeNode>()));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "1\n");
}

TEST(InterpreterControlFlowTest, ForConditionAssignTracksVariableValue)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "y", std::make_unique<ast::ValueNode>(6)));

    auto for_body = std::make_unique<ast::ScopeNode>();
    for_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1))));
    for_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("y"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("y"),
            std::make_unique<ast::ValueNode>(2))));

    root.add_statement(std::make_unique<ast::ForNode>(
        nullptr,
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("x"),
                                          std::make_unique<ast::VarNode>("y")),
        nullptr,
        std::move(for_body)));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("y")));

    EXPECT_EQ(RunAndCapture(root), "-6\n");
}

TEST(InterpreterControlFlowTest, ForConditionVarDeclTracksVariableValue)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "y", std::make_unique<ast::ValueNode>(6)));

    auto for_body = std::make_unique<ast::ScopeNode>();
    for_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1))));
    for_body->add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("y"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::sub,
            std::make_unique<ast::VarNode>("y"),
            std::make_unique<ast::ValueNode>(2))));

    root.add_statement(std::make_unique<ast::ForNode>(
        nullptr,
        std::make_unique<ast::VarDeclNode>("x",
                                           std::make_unique<ast::VarNode>("y")),
        nullptr,
        std::move(for_body)));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("y")));

    EXPECT_EQ(RunAndCapture(root), "-6\n");
}

TEST(InterpreterControlFlowTest, ForNodeMissingConditionThrows)
{
    ast::Interpreter interpreter;
    ast::ForNode node(
        nullptr, nullptr, nullptr, std::make_unique<ast::ScopeNode>());
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterControlFlowTest, ForNodeMissingBodyWithFalseConditionIsNoOp)
{
    ast::Interpreter interpreter;
    ast::ForNode node(
        nullptr, std::make_unique<ast::ValueNode>(0), nullptr, nullptr);
    EXPECT_NO_THROW(node.accept(interpreter));
}

TEST(InterpreterControlFlowTest, ForNodeHeaderScopeIsolationTest)
{
    ast::ScopeNode root;

    auto for_node = std::make_unique<ast::ForNode>(
        std::make_unique<ast::VarDeclNode>("i",
                                           std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("i"),
            std::make_unique<ast::ValueNode>(1)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("i"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("i"),
                std::make_unique<ast::ValueNode>(1))),
        std::make_unique<ast::ScopeNode>());

    root.add_statement(std::move(for_node));
    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("i")));

    ast::Interpreter interpreter;
    EXPECT_THROW(root.accept(interpreter), std::runtime_error);
}

TEST(InterpreterControlFlowTest, ScopeNodeShadowingTest)
{
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(1)));

    auto inner = std::make_unique<ast::ScopeNode>();
    inner->add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(2)));
    inner->add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));
    root.add_statement(std::move(inner));

    root.add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "2\n1\n");
}
