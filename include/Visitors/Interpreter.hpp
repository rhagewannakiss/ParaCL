#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "AST/AST.hpp"
#include "Visitors/Visitor.hpp"
#include "Visitors/detail/VarTable.hpp"

namespace ast {

class Interpreter : public Visitor
{
    using LoopInputNodes = std::unordered_set<const InputNode*>;
    using LoopInputCache = std::unordered_map<const InputNode*, int64_t>;

    VarTable table_;
    int64_t last_value_;
    std::vector<LoopInputNodes> loop_input_nodes_stack_;
    std::vector<LoopInputCache> loop_input_cache_stack_;

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
    void with_loop_input_context(const BaseNode& condition_root,
                                 const std::function<void()>& body);
    void push_loop_input_context(const BaseNode& condition_root);
    void pop_loop_input_context();
    std::optional<std::string> validate_evaluable_node(
        const BaseNode& node,
        const char* error_msg,
        evaluable_context context = evaluable_context::general) const;
};
} // namespace ast
