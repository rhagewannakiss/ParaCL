#include "AST/AST.hpp"
#include "Visitors/Interpreter.hpp"
#include "gtest/gtest.h"
#include <memory>
#include <sstream>
#include <stdexcept>

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

std::string CaptureRuntimeError(ast::BaseNode& node)
{
    ast::Interpreter interpreter;
    try {
        node.accept(interpreter);
    } catch (const std::runtime_error& ex) {
        return ex.what();
    }
    return "";
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

TEST(PrintNodeUnitTest, CheckNodeTypesInExpr)
{
    ast::Interpreter interpreter;

    {
        ast::PrintNode node(std::make_unique<ast::ScopeNode>());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(1)));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::WhileNode>(
            std::make_unique<ast::ValueNode>(0),
            std::make_unique<ast::ExprNode>(
                std::make_unique<ast::ValueNode>(1))));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ScopedCinInput input("12\n");
        ast::PrintNode node(std::make_unique<ast::InputNode>());
        EXPECT_NO_THROW({
            const std::string output = RunAndCapture(node);
            EXPECT_EQ(output, "12\n");
        });
    }

    {
        ast::PrintNode node(std::make_unique<ast::VarDeclNode>(
            "x", std::make_unique<ast::ValueNode>(1)));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::PrintNode>(
            std::make_unique<ast::ValueNode>(1)));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::PrintNode node(std::make_unique<ast::IfNode>(
            std::make_unique<ast::ValueNode>(1),
            std::make_unique<ast::ExprNode>(
                std::make_unique<ast::ValueNode>(1)),
            nullptr));
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(InputNodeUnitTest, CheckAvailableNodesForInput)
{
    {
        ScopedCinInput input("69\n");
        ast::PrintNode node(std::make_unique<ast::InputNode>());
        EXPECT_NO_THROW({
            const std::string output = RunAndCapture(node);
            EXPECT_EQ(output, "69\n");
        });
    }

    {
        ScopedCinInput input("abc\n");
        ast::Interpreter interpreter;
        ast::InputNode node;
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(DiagnosticsTest, RuntimeErrorsUseGnuFormat)
{
    ScopedCinInput input("abc\n");
    ast::InputNode node;

    ast::SourceRange range;
    range.file = "runtime.pcl";
    range.begin_line = 4;
    range.begin_column = 2;
    node.set_location(range);

    EXPECT_EQ(CaptureRuntimeError(node),
              "runtime.pcl:4:2: error: Input error: expected int64_t");
}

inline ast::BaseNode::NodePtr MakeThenExpr()
{
    return std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(1));
}

inline ast::BaseNode::NodePtr MakeWhileBodyScope()
{
    return std::make_unique<ast::ScopeNode>();
}

inline ast::BaseNode::NodePtr MakeForBodyScope()
{
    return std::make_unique<ast::ScopeNode>();
}

TEST(IfNodeUnitTest, CheckAvailableNodesForCondition)
{
    ast::Interpreter interpreter;

    {
        ast::IfNode node(std::make_unique<ast::AssignNode>(
                             std::make_unique<ast::VarNode>("x"),
                             std::make_unique<ast::ValueNode>(1)),
                         MakeThenExpr(),
                         nullptr);
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::IfNode node(std::make_unique<ast::PrintNode>(
                             std::make_unique<ast::ValueNode>(1)),
                         MakeThenExpr(),
                         nullptr);
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(
            std::make_unique<ast::ScopeNode>(), MakeThenExpr(), nullptr);
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::IfNode node(std::make_unique<ast::WhileNode>(
                             std::make_unique<ast::ValueNode>(0),
                             std::make_unique<ast::ExprNode>(
                                 std::make_unique<ast::ValueNode>(1))),
                         MakeThenExpr(),
                         nullptr);
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ScopedCinInput input("0\n");
        ast::IfNode node(
            std::make_unique<ast::InputNode>(), MakeThenExpr(), nullptr);
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::IfNode node(std::make_unique<ast::VarDeclNode>(
                             "x", std::make_unique<ast::ValueNode>(1)),
                         MakeThenExpr(),
                         nullptr);
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::IfNode node(std::make_unique<ast::IfNode>(
                             std::make_unique<ast::ValueNode>(0),
                             std::make_unique<ast::ExprNode>(
                                 std::make_unique<ast::ValueNode>(1)),
                             nullptr),
                         MakeThenExpr(),
                         nullptr);
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(WhileNodeUnitTest, CheckAvailableNodesForCondition)
{
    ast::Interpreter interpreter;

    {
        ast::WhileNode node(std::make_unique<ast::AssignNode>(
                                std::make_unique<ast::VarNode>("x_assign"),
                                std::make_unique<ast::ValueNode>(0)),
                            MakeWhileBodyScope());
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::WhileNode node(std::make_unique<ast::VarDeclNode>(
                                "x_decl", std::make_unique<ast::ValueNode>(0)),
                            MakeWhileBodyScope());
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ScopedCinInput input("0\n");
        ast::WhileNode node(std::make_unique<ast::InputNode>(),
                            MakeWhileBodyScope());
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::WhileNode node(std::make_unique<ast::PrintNode>(
                                std::make_unique<ast::ValueNode>(1)),
                            MakeWhileBodyScope());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(std::make_unique<ast::ScopeNode>(),
                            MakeWhileBodyScope());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(std::make_unique<ast::WhileNode>(
                                std::make_unique<ast::ValueNode>(0),
                                std::make_unique<ast::ScopeNode>()),
                            MakeWhileBodyScope());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::WhileNode node(std::make_unique<ast::IfNode>(
                                std::make_unique<ast::ValueNode>(0),
                                std::make_unique<ast::ExprNode>(
                                    std::make_unique<ast::ValueNode>(1)),
                                nullptr),
                            MakeWhileBodyScope());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(ForNodeUnitTest, CheckAvailableNodesForCondition)
{
    ast::Interpreter interpreter;

    {
        ast::ForNode node(nullptr,
                          std::make_unique<ast::AssignNode>(
                              std::make_unique<ast::VarNode>("x"),
                              std::make_unique<ast::ValueNode>(0)),
                          nullptr,
                          MakeForBodyScope());
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::ForNode node(nullptr,
                          std::make_unique<ast::VarDeclNode>(
                              "x", std::make_unique<ast::ValueNode>(0)),
                          nullptr,
                          MakeForBodyScope());
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ScopedCinInput input("0\n");
        ast::ForNode node(nullptr,
                          std::make_unique<ast::InputNode>(),
                          nullptr,
                          MakeForBodyScope());
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::ForNode node(nullptr,
                          std::make_unique<ast::PrintNode>(
                              std::make_unique<ast::ValueNode>(1)),
                          nullptr,
                          MakeForBodyScope());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }

    {
        ast::ForNode node(nullptr,
                          std::make_unique<ast::IfNode>(
                              std::make_unique<ast::ValueNode>(0),
                              std::make_unique<ast::ExprNode>(
                                  std::make_unique<ast::ValueNode>(1)),
                              nullptr),
                          nullptr,
                          MakeForBodyScope());
        EXPECT_THROW(node.accept(interpreter), std::runtime_error);
    }
}

TEST(ForNodeUnitTest, CheckAvailableNodesForBody)
{
    ast::Interpreter interpreter;

    {
        ast::ForNode node(nullptr,
                          std::make_unique<ast::ValueNode>(0),
                          nullptr,
                          std::make_unique<ast::ExprNode>(
                              std::make_unique<ast::ValueNode>(1)));
        EXPECT_NO_THROW(node.accept(interpreter));
    }

    {
        ast::ForNode node(nullptr,
                          std::make_unique<ast::ValueNode>(0),
                          nullptr,
                          std::make_unique<ast::AssignNode>(
                              std::make_unique<ast::VarNode>("x"),
                              std::make_unique<ast::ValueNode>(1)));
        EXPECT_NO_THROW(node.accept(interpreter));
    }
}
