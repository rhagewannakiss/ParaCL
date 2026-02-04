#include <fstream>
#include <memory>

#include "../include/AST.hpp"
#include "../include/DotVisitor.hpp"

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

    std::vector<BaseNode::NodePtr> stmts;
    stmts.push_back(std::move(assign_init));
    stmts.push_back(std::move(print_a));
    stmts.push_back(std::move(while_node));

    auto root = std::make_unique<ScopeNode>(std::move(stmts));
    AST ast(std::move(root));

    std::ofstream out("ast.dot");
    DotVisitor dv(out);
    dv.create_dot(ast);
}
