#include <cstdlib>
#include <fstream>
#include <filesystem>
#include <regex>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "../include/AST.hpp"
#include "../include/DotVisitor.hpp"

namespace {
bool check_node_equality_val(const ast::BaseNode* node1,
                             const ast::BaseNode* node2) 
{
    if (node1 == nullptr && node2 == nullptr) return true;
    if (node1 == nullptr && node2 != nullptr) return false;
    if (node1 != nullptr && node2 == nullptr) return false;
    
    auto type_1 = node1->node_type();
    auto type_2 = node2->node_type();
    
    if (type_1 != type_2) return false;
    switch (type_1) {
        case ast::base_node_type::value: 
            {
                auto* va = dynamic_cast<const ast::ValueNode*>(node1);
                auto* vb = dynamic_cast<const ast::ValueNode*>(node2);
                if (!va || !vb) return false;
                return va->value() == vb->value();
            }
        case ast::base_node_type::var: 
            {
                auto* va = dynamic_cast<const ast::VarNode*>(node1);
                auto* vb = dynamic_cast<const ast::VarNode*>(node2);
                if (!va || !vb) return false;
                return va->name() == vb->name();
            }
        case ast::base_node_type::unop: 
            {
                auto* va = dynamic_cast<const ast::UnOpNode*>(node1);
                auto* vb = dynamic_cast<const ast::UnOpNode*>(node2);
                if (!va || !vb) return false;
                bool ops_equal = check_node_equality_val(va->operand(), vb->operand());
                return va->op() == vb->op() && ops_equal;
            }
        case ast::base_node_type::bin_arith_op:
            {
                auto* va = dynamic_cast<const ast::BinArithOpNode*>(node1);
                auto* vb = dynamic_cast<const ast::BinArithOpNode*>(node2);
                if (!va || !vb) return false;
                bool ops_equal = check_node_equality_val(va->left(), vb->left()) &&
                                 check_node_equality_val(va->right(), vb->right());
                return va->op() == vb->op() && ops_equal;
            }
        case ast::base_node_type::bin_logic_op:
            {
                auto* va = dynamic_cast<const ast::BinLogicOpNode*>(node1);
                auto* vb = dynamic_cast<const ast::BinLogicOpNode*>(node2);
                if (!va || !vb) return false;
                bool ops_equal = check_node_equality_val(va->left(), vb->left()) &&
                                 check_node_equality_val(va->right(), vb->right());
                return va->op() == vb->op() && ops_equal;
            }
        case ast::base_node_type::print:
            {
                auto *va = dynamic_cast<const ast::PrintNode*>(node1);
                auto *vb = dynamic_cast<const ast::PrintNode*>(node2);
                if (!va || !vb) return false;
                return check_node_equality_val(va->expr(), vb->expr());
            }
        case ast::base_node_type::assign:
            {
                auto *va = dynamic_cast<const ast::AssignNode*>(node1);
                auto *vb = dynamic_cast<const ast::AssignNode*>(node2);
                if (!va || !vb) return false;
                return check_node_equality_val(va->lhs(), vb->lhs()) &&
                       check_node_equality_val(va->rhs(), vb->rhs());
            }
        case ast::base_node_type::expr:
            {
                auto* va = dynamic_cast<const ast::ExprNode*>(node1);
                auto* vb = dynamic_cast<const ast::ExprNode*>(node2);
                if (!va || !vb) return false;
                return check_node_equality_val(va->expr(), vb->expr());
            }
        case ast::base_node_type::if_node:
            {
                auto* va = dynamic_cast<const ast::IfNode*>(node1);
                auto* vb = dynamic_cast<const ast::IfNode*>(node2);
                if (!va || !vb) return false;
                return check_node_equality_val(va->condition(), vb->condition()) &&
                       check_node_equality_val(va->then_branch(), vb->then_branch()) &&
                       check_node_equality_val(va->else_branch(), vb->else_branch());
            }
        case ast::base_node_type::while_node:
            {
                auto* va = dynamic_cast<const ast::WhileNode*>(node1);
                auto* vb = dynamic_cast<const ast::WhileNode*>(node2);
                if (!va || !vb) return false;
                return check_node_equality_val(va->condition(), vb->condition()) &&
                       check_node_equality_val(va->body(), vb->body());
            }
        case ast::base_node_type::input:
            {
                auto* va = dynamic_cast<const ast::InputNode*>(node1);
                auto* vb = dynamic_cast<const ast::InputNode*>(node2);
                if (!va || !vb) return false;
                return check_node_equality_val(va->lhs(), vb->lhs());
            }
        case ast::base_node_type::scope:
            {
                auto* va = dynamic_cast<const ast::ScopeNode*>(node1);
                auto* vb = dynamic_cast<const ast::ScopeNode*>(node2);
                if (!va || !vb) return false;
                const auto& ca = va->statements();
                const auto& cb = vb->statements();
                if (ca.size() != cb.size()) return false;
                for (size_t i = 0; i < ca.size(); ++i) {
                    if (!check_node_equality_val(ca[i].get(), cb[i].get())) 
                        return false;
                }
                return true;
            }
        case ast::base_node_type::base:
            assert(false);
        default:
            break;
    }
    return false;
}

enum class tst //test return types 
{
    equal,
    same_pointers,
    not_equal, 
    null_pointer
};

tst check_equality_values(const ast::AST& ast1, 
                          const ast::AST& ast2)
{
    if (!ast1.root() || !ast2.root()) {
        return tst::null_pointer;
    }
    if(ast1.root() == ast2.root()) {
        return tst::same_pointers; 
    }

    if(check_node_equality_val(ast1.root(), ast2.root())) {
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

    auto var_a1 = std::make_unique<ast::VarNode>("a");
    auto add    = std::make_unique<ast::BinArithOpNode>(
        ast::bin_arith_op_type::add,
        std::move(lit1),
        std::move(lit2));
    auto assign_init = std::make_unique<ast::AssignNode>(
        std::move(var_a1), 
        std::move(add));

    auto var_a2 =  std::make_unique<ast::VarNode>("a");
    auto print_a = std::make_unique<ast::PrintNode>(
        std::move(var_a2));

    auto var_a3 = std::make_unique<ast::VarNode>("a");
    auto lit10 =  std::make_unique<ast::ValueNode>(10);
    auto cond =   std::make_unique<ast::BinLogicOpNode>(
        ast::bin_logic_op_type::less,
        std::move(var_a3),
        std::move(lit10));

    auto var_a4 =   std::make_unique<ast::VarNode>("a");
    auto lit1_inc = std::make_unique<ast::ValueNode>(1);
    auto add_inc =  std::make_unique<ast::BinArithOpNode>(
        ast::bin_arith_op_type::add,
        std::move(var_a4),
        std::move(lit1_inc));
    
    auto assign_inc = std::make_unique<ast::AssignNode>(
        std::make_unique<ast::VarNode>("a"), 
        std::move(add_inc));

    auto while_body = std::make_unique<ast::ScopeNode>();
    while_body->add_statement(std::move(assign_inc));

    auto while_node = std::make_unique<ast::WhileNode>(
        std::move(cond), 
        std::move(while_body));

    std::vector<ast::BaseNode::NodePtr> stmts;
    stmts.push_back(std::move(assign_init));
    stmts.push_back(std::move(print_a));
    stmts.push_back(std::move(while_node));

    auto root = std::make_unique<ast::ScopeNode>(
        std::move(stmts));
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

    auto convert_to_png = [](const std::string& dot_file, const std::string& png_file) {
        std::string cmd = "dot -Tpng " + dot_file + " -o " + png_file;
        int rc = std::system(cmd.c_str());
        return rc == 0;
    };
    ASSERT_TRUE(convert_to_png("dots/ast.dot", "pngs/ast.png"));
    ASSERT_TRUE(convert_to_png("dots/ast_cloned.dot", "pngs/ast_cloned.png"));

    auto dot_original = strip_addresses(read_file("dots/ast.dot"));
    auto dot_cloned   = strip_addresses(read_file("dots/ast_cloned.dot"));

    EXPECT_NE(ast.root(), cloned_ast.root());
    EXPECT_EQ(check_equality_values(ast, cloned_ast), tst::equal);
    EXPECT_EQ(dot_original, dot_cloned);
}

TEST(ASTCloneTest, AssignmentIndependence)
{
    auto val = std::make_unique<ast::ValueNode>(5);
    auto var = std::make_unique<ast::VarNode>("x");
    auto assign = std::make_unique<ast::AssignNode>(
        std::move(var),
        std::move(val));
    std::vector<ast::BaseNode::NodePtr> stmts;
    stmts.push_back(std::move(assign));
    auto root = std::make_unique<ast::ScopeNode>(std::move(stmts));
    ast::AST original(std::move(root));

    ast::AST assigned;
    assigned = original;

    EXPECT_NE(original.root(), assigned.root());
    EXPECT_EQ(check_equality_values(original, assigned), tst::equal);
}
