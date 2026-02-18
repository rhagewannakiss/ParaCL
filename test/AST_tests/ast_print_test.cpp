#include <filesystem>
#include <fstream>
#include <memory>

#include "AST/AST.hpp"
#include "Visitors/DotVisitor.hpp"

using namespace ast;

int main()
{
    auto lit1 = std::make_unique<ValueNode>(1);
    auto lit2 = std::make_unique<ValueNode>(2);

    auto var_a1 = std::make_unique<VarNode>("a");
    auto add = std::make_unique<BinArithOpNode>(
        bin_arith_op_type::add,
        std::move(lit1),
        std::move(lit2));
    auto assign_init =
        std::make_unique<AssignNode>(std::move(var_a1), 
                                     std::move(add));

    auto var_a2 = std::make_unique<VarNode>("a");
    auto print_a = std::make_unique<PrintNode>(std::move(var_a2));

    auto var_a3 = std::make_unique<VarNode>("a");
    auto lit10 = std::make_unique<ValueNode>(10);
    auto cond = std::make_unique<BinLogicOpNode>(
        bin_logic_op_type::less,
        std::move(var_a3),
        std::move(lit10));

    auto var_a4 = std::make_unique<VarNode>("a");
    auto lit1_inc = std::make_unique<ValueNode>(1);
    auto add_inc = std::make_unique<BinArithOpNode>(
        bin_arith_op_type::add,
        std::move(var_a4),
        std::move(lit1_inc));

    auto assign_inc = std::make_unique<AssignNode>(
        std::make_unique<VarNode>("a"),
        std::move(add_inc));

    auto while_body = std::make_unique<ScopeNode>();
    while_body->add_statement(std::move(assign_inc));

    auto while_node = std::make_unique<WhileNode>(
        std::move(cond),
        std::move(while_body));

    auto for_body = std::make_unique<ScopeNode>();
    for_body->add_statement(std::make_unique<PrintNode>(
        std::make_unique<VarNode>("i")));

    auto for_init = std::make_unique<VarDeclNode>(
        "i",
        std::make_unique<ValueNode>(0));

    auto for_cond = std::make_unique<BinLogicOpNode>(
        bin_logic_op_type::less,
        std::make_unique<VarNode>("i"),
        std::make_unique<ValueNode>(3));

    auto for_step_expr = std::make_unique<BinArithOpNode>(
        bin_arith_op_type::add,
        std::make_unique<VarNode>("i"),
        std::make_unique<ValueNode>(1));
    auto for_step = std::make_unique<AssignNode>(
        std::make_unique<VarNode>("i"),
        std::move(for_step_expr));

    auto for_node = std::make_unique<ForNode>(
        std::move(for_init),
        std::move(for_cond),
        std::move(for_step),
        std::move(for_body));

    std::vector<BaseNode::NodePtr> stmts;
    stmts.push_back(std::move(assign_init));
    stmts.push_back(std::move(print_a));
    stmts.push_back(std::move(while_node));
    stmts.push_back(std::move(for_node));

    auto root = std::make_unique<ScopeNode>(std::move(stmts));
    AST ast(std::move(root));

    std::filesystem::create_directories("dump");
    std::ofstream out("dump/ast.dot");
    DotVisitor dv(out);
    dv.create_dot(ast);
    out.flush();

    int res = std::system("dot -Tpng dump/ast.dot -o dump/ast.png");
    return res == 0 ? 0 : 1;
}
