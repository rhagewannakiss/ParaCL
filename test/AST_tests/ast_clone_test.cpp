#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include "AST/AST.hpp"
#include "Visitors/DotVisitor.hpp"
#include "gtest/gtest.h"

namespace {

template<typename T>
struct const_two_ptrs
{
    const T* va;
    const T* vb;
    const bool is_valid;
};

template<typename T>
const_two_ptrs<T> check_ptr_equality(const ast::BaseNode* node1,
                                     const ast::BaseNode* node2)
{
    const T* va = dynamic_cast<const T*>(node1);
    const T* vb = dynamic_cast<const T*>(node2);
    if (!va || !vb)
        return { nullptr, nullptr, false };
    return { va, vb, true };
}

bool check_node_equality(const ast::BaseNode* node1,
                         const ast::BaseNode* node2);

template<typename T, typename Comparator>
bool compare_typed_nodes(const ast::BaseNode* node1,
                         const ast::BaseNode* node2,
                         Comparator comparator)
{
    const auto ptrs = check_ptr_equality<T>(node1, node2);
    if (!ptrs.is_valid) {
        return false;
    }
    return comparator(ptrs.va, ptrs.vb);
}

template<typename T>
bool compare_leaf_nodes(const ast::BaseNode* node1, const ast::BaseNode* node2)
{
    return compare_typed_nodes<T>(
        node1, node2, [](const T*, const T*) { return true; });
}

template<typename T>
bool compare_unary_nodes(const ast::BaseNode* node1, const ast::BaseNode* node2)
{
    return compare_typed_nodes<T>(node1, node2, [](const T* a, const T* b) {
        return a->op() == b->op() &&
               check_node_equality(a->operand(), b->operand());
    });
}

template<typename T>
bool compare_binary_nodes(const ast::BaseNode* node1,
                          const ast::BaseNode* node2)
{
    return compare_typed_nodes<T>(node1, node2, [](const T* a, const T* b) {
        return a->op() == b->op() &&
               check_node_equality(a->left(), b->left()) &&
               check_node_equality(a->right(), b->right());
    });
}

bool check_node_equality(const ast::BaseNode* node1, const ast::BaseNode* node2)
{
    if (node1 == nullptr || node2 == nullptr)
        return node1 == node2;

    auto type_1 = node1->node_type();
    auto type_2 = node2->node_type();

    if (type_1 != type_2)
        return false;
    switch (type_1) {
        case ast::base_node_type::value:
            return compare_typed_nodes<ast::ValueNode>(
                node1,
                node2,
                [](const ast::ValueNode* a, const ast::ValueNode* b) {
                    return a->value() == b->value();
                });
        case ast::base_node_type::var:
            return compare_typed_nodes<ast::VarNode>(
                node1, node2, [](const ast::VarNode* a, const ast::VarNode* b) {
                    return a->name() == b->name();
                });
        case ast::base_node_type::unop:
            return compare_unary_nodes<ast::UnOpNode>(node1, node2);
        case ast::base_node_type::bin_arith_op:
            return compare_binary_nodes<ast::BinArithOpNode>(node1, node2);
        case ast::base_node_type::bin_logic_op:
            return compare_binary_nodes<ast::BinLogicOpNode>(node1, node2);
        case ast::base_node_type::print:
            return compare_typed_nodes<ast::PrintNode>(
                node1,
                node2,
                [](const ast::PrintNode* a, const ast::PrintNode* b) {
                    return check_node_equality(a->expr(), b->expr());
                });
        case ast::base_node_type::assign:
            return compare_typed_nodes<ast::AssignNode>(
                node1,
                node2,
                [](const ast::AssignNode* a, const ast::AssignNode* b) {
                    return check_node_equality(a->lhs(), b->lhs()) &&
                           check_node_equality(a->rhs(), b->rhs());
                });
        case ast::base_node_type::expr:
            return compare_typed_nodes<ast::ExprNode>(
                node1,
                node2,
                [](const ast::ExprNode* a, const ast::ExprNode* b) {
                    return check_node_equality(a->expr(), b->expr());
                });
        case ast::base_node_type::if_node:
            return compare_typed_nodes<ast::IfNode>(
                node1, node2, [](const ast::IfNode* a, const ast::IfNode* b) {
                    return check_node_equality(a->condition(),
                                               b->condition()) &&
                           check_node_equality(a->then_branch(),
                                               b->then_branch()) &&
                           check_node_equality(a->else_branch(),
                                               b->else_branch());
                });
        case ast::base_node_type::while_node:
            return compare_typed_nodes<ast::WhileNode>(
                node1,
                node2,
                [](const ast::WhileNode* a, const ast::WhileNode* b) {
                    return check_node_equality(a->condition(),
                                               b->condition()) &&
                           check_node_equality(a->body(), b->body());
                });
        case ast::base_node_type::for_node:
            return compare_typed_nodes<ast::ForNode>(
                node1, node2, [](const ast::ForNode* a, const ast::ForNode* b) {
                    return check_node_equality(a->get_init(), b->get_init()) &&
                           check_node_equality(a->get_cond(), b->get_cond()) &&
                           check_node_equality(a->get_step(), b->get_step()) &&
                           check_node_equality(a->get_body(), b->get_body());
                });
        case ast::base_node_type::input:
            return compare_leaf_nodes<ast::InputNode>(node1, node2);
        case ast::base_node_type::scope:
            return compare_typed_nodes<ast::ScopeNode>(
                node1,
                node2,
                [](const ast::ScopeNode* a, const ast::ScopeNode* b) {
                    const auto& ca = a->statements();
                    const auto& cb = b->statements();
                    if (ca.size() != cb.size())
                        return false;
                    for (size_t i = 0; i < ca.size(); ++i) {
                        if (!check_node_equality(ca[i].get(), cb[i].get()))
                            return false;
                    }
                    return true;
                });
        case ast::base_node_type::var_decl:
            return compare_typed_nodes<ast::VarDeclNode>(
                node1,
                node2,
                [](const ast::VarDeclNode* a, const ast::VarDeclNode* b) {
                    if (a->name() != b->name())
                        return false;
                    return check_node_equality(a->init_expr(), b->init_expr());
                });
        case ast::base_node_type::err:
            return compare_leaf_nodes<ast::ErrorNode>(node1, node2);
        case ast::base_node_type::empty:
            return compare_leaf_nodes<ast::EmptyNode>(node1, node2);
        case ast::base_node_type::base:
        default:
            return false;
    }
    return false;
}

enum class tst // test return types
{
    equal,
    same_pointers,
    not_equal,
    null_pointer
};

tst check_equality_values(const ast::AST& ast1, const ast::AST& ast2)
{
    if (!ast1.root() || !ast2.root()) {
        return tst::null_pointer;
    }
    if (ast1.root() == ast2.root()) {
        return tst::same_pointers;
    }

    if (check_node_equality(ast1.root(), ast2.root())) {
        return tst::equal;
    }

    return tst::not_equal;
}

std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::string strip_addresses(const std::string& dot_contents)
{
    std::regex addr(R"(0x[0-9a-fA-F]+)");
    return std::regex_replace(dot_contents, addr, "ADDR");
}
} // namespace

TEST(ASTCloneTest, Test1)
{
    auto lit1 = std::make_unique<ast::ValueNode>(1);
    auto lit2 = std::make_unique<ast::ValueNode>(2);

    auto add = std::make_unique<ast::BinArithOpNode>(
        ast::bin_arith_op_type::add, std::move(lit1), std::move(lit2));
    auto decl_a = std::make_unique<ast::VarDeclNode>("a", std::move(add));
    auto decl_b = std::make_unique<ast::VarDeclNode>("b");

    auto var_a2 = std::make_unique<ast::VarNode>("a");
    auto print_a = std::make_unique<ast::PrintNode>(std::move(var_a2));

    auto var_a3 = std::make_unique<ast::VarNode>("a");
    auto lit10 = std::make_unique<ast::ValueNode>(10);
    auto cond = std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::less, std::move(var_a3), std::move(lit10));

    auto var_a4 = std::make_unique<ast::VarNode>("a");
    auto lit1_inc = std::make_unique<ast::ValueNode>(1);
    auto add_inc = std::make_unique<ast::BinArithOpNode>(
        ast::bin_arith_op_type::add, std::move(var_a4), std::move(lit1_inc));

    auto assign_inc = std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("a"), std::move(add_inc));

    auto while_body = std::make_unique<ast::ScopeNode>();
    while_body->add_statement(std::move(assign_inc));

    auto while_node = std::make_unique<ast::WhileNode>(std::move(cond),
                                                       std::move(while_body));

    auto for_init =
        std::make_unique<ast::AssignNode>(std::make_unique<ast::VarNode>("b"),
                                          std::make_unique<ast::ValueNode>(0));

    auto for_cond = std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::less,
        std::make_unique<ast::VarNode>("b"),
        std::make_unique<ast::ValueNode>(3));

    auto for_step = std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("b"),
        std::make_unique<ast::BinArithOpNode>(
            ast::bin_arith_op_type::add,
            std::make_unique<ast::VarNode>("b"),
            std::make_unique<ast::ValueNode>(1)));

    auto for_body = std::make_unique<ast::ScopeNode>();
    for_body->add_statement(
        std::make_unique<ast::PrintNode>(std::make_unique<ast::VarNode>("b")));

    auto for_node = std::make_unique<ast::ForNode>(std::move(for_init),
                                                   std::move(for_cond),
                                                   std::move(for_step),
                                                   std::move(for_body));

    std::vector<ast::BaseNode::NodePtr> stmts;
    stmts.push_back(std::move(decl_a));
    stmts.push_back(std::move(decl_b));
    stmts.push_back(std::move(print_a));
    stmts.push_back(std::move(while_node));
    stmts.push_back(std::move(for_node));

    auto root = std::make_unique<ast::ScopeNode>(std::move(stmts));
    ast::AST ast(std::move(root));

    auto cloned_ast(ast);

    std::filesystem::create_directories("dots");
    std::filesystem::create_directories("pngs");

    std::ofstream out("dots/ast.dot");
    ast::DotVisitor dv(out);
    dv.begin_graph();
    if (ast.root()) {
        ast.root()->accept(dv);
    }
    dv.end_graph();

    std::ofstream out_cloned("dots/ast_cloned.dot");
    ast::DotVisitor dv_cloned(out_cloned);
    dv_cloned.begin_graph();
    if (cloned_ast.root()) {
        cloned_ast.root()->accept(dv_cloned);
    }
    dv_cloned.end_graph();

    out.close();
    out_cloned.close();

    auto convert_to_png = [](const std::string& dot_file,
                             const std::string& png_file) {
        std::string cmd = "dot -Tpng " + dot_file + " -o " + png_file;
        int rc = std::system(cmd.c_str());
        return rc == 0;
    };
    ASSERT_TRUE(convert_to_png("dots/ast.dot", "pngs/ast.png"));
    ASSERT_TRUE(convert_to_png("dots/ast_cloned.dot", "pngs/ast_cloned.png"));

    auto dot_original = strip_addresses(read_file("dots/ast.dot"));
    auto dot_cloned = strip_addresses(read_file("dots/ast_cloned.dot"));

    EXPECT_NE(ast.root(), cloned_ast.root());
    EXPECT_EQ(check_equality_values(ast, cloned_ast), tst::equal);
    EXPECT_EQ(dot_original, dot_cloned);
}

TEST(ASTCloneTest, AssignmentIndependence)
{
    auto val = std::make_unique<ast::ValueNode>(5);
    auto var = std::make_unique<ast::VarNode>("x");
    auto assign =
        std::make_unique<ast::AssignNode>(std::move(var), std::move(val));
    std::vector<ast::BaseNode::NodePtr> stmts;
    stmts.push_back(std::move(assign));
    auto root = std::make_unique<ast::ScopeNode>(std::move(stmts));
    ast::AST original(std::move(root));

    ast::AST assigned;
    assigned = original;

    EXPECT_NE(original.root(), assigned.root());
    EXPECT_EQ(check_equality_values(original, assigned), tst::equal);
}

TEST(ASTCloneTest, ErrorAndEmptyNodesCloneEquality)
{
    std::vector<ast::BaseNode::NodePtr> stmts;
    stmts.push_back(std::make_unique<ast::ErrorNode>());
    stmts.push_back(std::make_unique<ast::EmptyNode>());

    auto root = std::make_unique<ast::ScopeNode>(std::move(stmts));
    ast::AST ast(std::move(root));
    ast::AST cloned(ast);

    EXPECT_NE(ast.root(), cloned.root());
    EXPECT_EQ(check_equality_values(ast, cloned), tst::equal);
}
