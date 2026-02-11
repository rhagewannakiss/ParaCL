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

TEST(InterpreterShortCircuitTest, LogicalAndSkipsRightWhenLeftIsZero) {
    ast::PrintNode node(std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::logical_and,
        std::make_unique<ast::ValueNode>(0),
        std::make_unique<ast::InputNode>(std::make_unique<ast::ValueNode>(1))
    ));

    EXPECT_NO_THROW({
        const std::string output = RunAndCapture(node);
        EXPECT_EQ(output, "0\n");
    });
}

TEST(InterpreterShortCircuitTest, LogicalOrSkipsRightWhenLeftIsNonZero) {
    ast::PrintNode node(std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::logical_or,
        std::make_unique<ast::ValueNode>(1),
        std::make_unique<ast::InputNode>(std::make_unique<ast::ValueNode>(1))
    ));

    EXPECT_NO_THROW({
        const std::string output = RunAndCapture(node);
        EXPECT_EQ(output, "1\n");
    });
}

TEST(InterpreterShortCircuitTest, LogicalAndEvaluatesRightWhenLeftIsNonZero) {
    ast::Interpreter interpreter;
    ast::PrintNode node(std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::logical_and,
        std::make_unique<ast::ValueNode>(1),
        std::make_unique<ast::InputNode>(std::make_unique<ast::ValueNode>(1))
    ));

    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}

TEST(InterpreterShortCircuitTest, LogicalOrEvaluatesRightWhenLeftIsZero) {
    ast::Interpreter interpreter;
    ast::PrintNode node(std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::logical_or,
        std::make_unique<ast::ValueNode>(0),
        std::make_unique<ast::InputNode>(std::make_unique<ast::ValueNode>(1))
    ));

    EXPECT_THROW(node.accept(interpreter), std::runtime_error);
}
