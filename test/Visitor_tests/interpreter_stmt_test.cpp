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

class ScopedCinInput {
public:
    explicit ScopedCinInput(const std::string& input) : in_(input), old_(std::cin.rdbuf(in_.rdbuf())) {}
    ~ScopedCinInput() { std::cin.rdbuf(old_); }

    ScopedCinInput(const ScopedCinInput&) = delete;
    ScopedCinInput& operator=(const ScopedCinInput&) = delete;

private:
    std::istringstream in_;
    std::streambuf* old_;
};
} // namespace

TEST(InterpreterStmtTest, VarDeclNodeWithInitTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(11)));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "11\n");
}

TEST(InterpreterStmtTest, VarDeclNodeWithoutInitTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>("x"));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "0\n");
}

TEST(InterpreterStmtTest, VarDeclDuplicateInSameScopeThrows) {
    ast::Interpreter interpreter;
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>("x"));
    root.add_statement(std::make_unique<ast::VarDeclNode>("x"));

    EXPECT_THROW(root.accept(interpreter), std::runtime_error);
}

TEST(InterpreterStmtTest, AssignNodeExistingVariableTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>(
        "x",
        std::make_unique<ast::ValueNode>(1)));
    root.add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::ValueNode>(9)));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "9\n");
}

TEST(InterpreterStmtTest, AssignNodeCreatesVariableWhenMissingTest) {
    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("x"),
        std::make_unique<ast::ValueNode>(5)));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "5\n");
}

TEST(InterpreterStmtTest, AssignNodeInvalidLhsThrows) {
    ast::Interpreter interpreter;
    ast::AssignNode node(
        std::make_unique<ast::ValueNode>(1),
        std::make_unique<ast::ValueNode>(2));

    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterStmtTest, AssignNodeMissingRhsThrows) {
    ast::Interpreter interpreter;
    ast::AssignNode node(std::make_unique<ast::VarNode>("x"), nullptr);

    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterStmtTest, InputNodeReadsIntegerTest) {
    ScopedCinInput input("42\n");

    ast::ScopeNode root;
    root.add_statement(std::make_unique<ast::VarDeclNode>("x"));
    root.add_statement(std::make_unique<ast::InputNode>(
        std::make_unique<ast::VarNode>("x")));
    root.add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("x")));

    EXPECT_EQ(RunAndCapture(root), "42\n");
}

TEST(InterpreterStmtTest, InputNodeInvalidInputThrows) {
    ScopedCinInput input("abc\n");

    ast::Interpreter interpreter;
    ast::InputNode node(std::make_unique<ast::VarNode>("x"));
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterStmtTest, PrintNodeValidExprTest) {
    ast::PrintNode node(std::make_unique<ast::ValueNode>(77));
    EXPECT_EQ(RunAndCapture(node), "77\n");
}

TEST(InterpreterStmtTest, PrintNodeMissingExprThrows) {
    ast::Interpreter interpreter;
    ast::PrintNode node;
    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}
