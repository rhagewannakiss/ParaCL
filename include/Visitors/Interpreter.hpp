#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "AST/AST.hpp"
#include "Visitors/Visitor.hpp"
#include "Visitors/detail/VarTable.hpp"

namespace ast {

class Interpreter : public Visitor
{

    VarTable table_;
    int64_t last_value_;

public:
    Interpreter();

    void visit(BinArithOpNode& node) override;
    void visit(BinLogicOpNode& node) override;
    void visit(ValueNode& node) override;
    void visit(UnOpNode& node) override;
    void visit(AssignNode& node) override;
    void visit(VarNode& node) override;
    void visit(IfNode& node) override;
    void visit(WhileNode& node) override;
    void visit(ForNode& node) override;
    void visit(InputNode& node) override;
    void visit(ExprNode& node) override;
    void visit(PrintNode& node) override;
    void visit(ScopeNode& node) override;
    void visit(VarDeclNode& node) override;
    void visit(ErrorNode& node) override;
    void visit(EmptyNode& node) override;

private:
    enum class evaluable_context
    {
        general,
        condition,
    };

    static bool add_overflow(int64_t lhs, int64_t rhs, int64_t& out);
    static bool sub_overflow(int64_t lhs, int64_t rhs, int64_t& out);
    static bool mul_overflow(int64_t lhs, int64_t rhs, int64_t& out);
    void evaluate_loop_condition(
        BaseNode& condition,
        const std::optional<std::string>& tracked_var_name,
        bool initialize_tracked_var);
    static std::optional<std::string> validate_evaluable_node(
        const BaseNode& node,
        const char* error_msg,
        evaluable_context context = evaluable_context::general);
};

} // namespace ast
