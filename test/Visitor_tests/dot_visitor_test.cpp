#include "AST/AST.hpp"
#include "Visitors/DotVisitor.hpp"
#include "gtest/gtest.h"
#include <regex>
#include <sstream>

namespace {
std::string StripAddresses(const std::string& dot) {
    static const std::regex addr_regex(R"(0x[0-9a-fA-F]+)");
    return std::regex_replace(dot, addr_regex, "ADDR");
}

std::string MakeDot(ast::AST& ast_tree) {
    std::ostringstream out;
    ast::DotVisitor visitor(out);
    visitor.create_dot(ast_tree);
    return StripAddresses(out.str());
}
} // namespace

TEST(DotVisitorTest, EmptyAstEmitsEmptyGraph) {
    ast::AST ast_tree;
    const std::string dot = MakeDot(ast_tree);

    EXPECT_EQ(dot, "digraph AST {\n}\n");
}

TEST(DotVisitorTest, ValueNodeGraphContainsValuePayload) {
    ast::AST ast_tree(std::make_unique<ast::ValueNode>(42));
    const std::string dot = MakeDot(ast_tree);

    EXPECT_NE(dot.find("digraph AST {\n"), std::string::npos);
    EXPECT_NE(dot.find("value\\n42\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("shape=box"), std::string::npos);
}

TEST(DotVisitorTest, BinArithNodeGraphContainsEdgesAndOperator) {
    ast::AST ast_tree(std::make_unique<ast::BinArithOpNode>(
        ast::bin_arith_op_type::add,
        std::make_unique<ast::ValueNode>(1),
        std::make_unique<ast::ValueNode>(2)));
    const std::string dot = MakeDot(ast_tree);

    EXPECT_NE(dot.find("bin_arith_op\\n+\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("value\\n1\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("value\\n2\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("n0 -> n1"), std::string::npos);
    EXPECT_NE(dot.find("n0 -> n2"), std::string::npos);
}

TEST(DotVisitorTest, ComplexTreeContainsControlFlowAndVarDeclLabels) {
    auto root = std::make_unique<ast::ScopeNode>();

    root->add_statement(std::make_unique<ast::VarDeclNode>(
        "x", std::make_unique<ast::ValueNode>(5)));

    root->add_statement(std::make_unique<ast::IfNode>(
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::greater,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("x")),
        nullptr));

    root->add_statement(std::make_unique<ast::WhileNode>(
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::ValueNode>(10)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("x"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("x"),
                std::make_unique<ast::ValueNode>(1)))));

    ast::AST ast_tree(std::move(root));
    const std::string dot = MakeDot(ast_tree);

    EXPECT_NE(dot.find("scope\\nscope\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("var_decl\\nvar_decl x\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("if\\nif\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("while\\nwhile\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("assign\\n=\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("print\\nprint\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("n0 -> n1"), std::string::npos);
}

TEST(DotVisitorTest, ForNodeGraphContainsForLabelAndEdges) {
    auto root = std::make_unique<ast::ScopeNode>();

    auto for_body = std::make_unique<ast::ScopeNode>();
    for_body->add_statement(std::make_unique<ast::PrintNode>(
        std::make_unique<ast::VarNode>("i")));

    root->add_statement(std::make_unique<ast::ForNode>(
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("i"),
            std::make_unique<ast::ValueNode>(0)),
        std::make_unique<ast::BinLogicOpNode>(
            ast::bin_logic_op_type::less,
            std::make_unique<ast::VarNode>("i"),
            std::make_unique<ast::ValueNode>(2)),
        std::make_unique<ast::AssignNode>(
            std::make_unique<ast::VarNode>("i"),
            std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::add,
                std::make_unique<ast::VarNode>("i"),
                std::make_unique<ast::ValueNode>(1))),
        std::move(for_body)));

    ast::AST ast_tree(std::move(root));
    const std::string dot = MakeDot(ast_tree);

    EXPECT_NE(dot.find("for\\nfor\\nADDR"), std::string::npos);
    EXPECT_NE(dot.find("n0 -> n1"), std::string::npos);
}
