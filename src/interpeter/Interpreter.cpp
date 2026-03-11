#include "Visitors/Interpreter.hpp"
#include "AST/AST.hpp"
#include "Visitors/detail/ScopeGuard.hpp"
#include "errors-output/error-formatter.hpp"

#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

namespace {

inline bool is_missing_node(const ast::BaseNode* node)
{
    return node == nullptr;
}

inline bool is_empty_node(const ast::BaseNode* node)
{
    return node != nullptr && node->node_type() == ast::base_node_type::empty;
}

inline bool is_missing_or_empty_expr_node(const ast::BaseNode* node)
{
    return is_missing_node(node) || is_empty_node(node);
}

inline bool is_missing_or_empty_stmt_node(const ast::BaseNode* node)
{
    return is_missing_node(node) || is_empty_node(node);
}

inline bool has_expr_node(const ast::BaseNode* node)
{
    return !is_missing_or_empty_expr_node(node);
}

inline bool has_stmt_node(const ast::BaseNode* node)
{
    return !is_missing_or_empty_stmt_node(node);
}

void require_expr_node(const ast::BaseNode* node,
                       const ast::SourceRange& owner_loc,
                       const char* error_msg)
{
    if (is_missing_or_empty_expr_node(node)) {
        throw std::runtime_error(err::format_error(owner_loc, error_msg));
    }
}

void accept_stmt_if_present(ast::BaseNode* node, ast::Visitor& visitor)
{
    if (has_stmt_node(node)) {
        node->accept(visitor);
    }
}

int64_t read_input_int64_or_throw(const ast::SourceRange& location)
{
    int64_t value = 0;
    if (!(std::cin >> value)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::runtime_error(
            err::format_error(location, "Input error: expected int64_t"));
    }
    return value;
}

} // namespace

namespace ast {

Interpreter::Interpreter()
  : last_value_(0)
{
}

void Interpreter::visit(BinArithOpNode& node)
{
    auto* left = node.left();
    auto* right = node.right();
    require_expr_node(left, node.location(), "BinArithOpNode missing operand");
    require_expr_node(right, node.location(), "BinArithOpNode missing operand");

    left->accept(*this);
    int64_t left_res = last_value_;
    right->accept(*this);
    int64_t right_res = last_value_;
    int64_t checked_result = 0;
    switch (node.op()) {
        case bin_arith_op_type::add:
            if (add_overflow(left_res, right_res, checked_result)) {
                throw std::runtime_error(err::format_error(
                    node.location(), "Integer overflow in addition"));
            }
            last_value_ = checked_result;
            break;
        case bin_arith_op_type::sub:
            if (sub_overflow(left_res, right_res, checked_result)) {
                throw std::runtime_error(err::format_error(
                    node.location(), "Integer overflow in subtraction"));
            }
            last_value_ = checked_result;
            break;
        case bin_arith_op_type::mul:
            if (mul_overflow(left_res, right_res, checked_result)) {
                throw std::runtime_error(err::format_error(
                    node.location(), "Integer overflow in multiplication"));
            }
            last_value_ = checked_result;
            break;
        case bin_arith_op_type::div:
            if (right_res == 0) {
                throw std::runtime_error(
                    err::format_error(node.location(), "Division by zero"));
            }
            if (left_res == std::numeric_limits<int64_t>::min() &&
                right_res == -1) {
                throw std::runtime_error(err::format_error(
                    node.location(), "Integer overflow in division"));
            }
            last_value_ = left_res / right_res;
            break;
        case ast::bin_arith_op_type::mod:
            if (right_res == 0) {
                throw std::runtime_error(
                    err::format_error(node.location(), "Division by zero"));
            }
            if (left_res == std::numeric_limits<int64_t>::min() &&
                right_res == -1) {
                throw std::runtime_error(err::format_error(
                    node.location(), "Integer overflow in modulus"));
            }
            last_value_ = left_res % right_res;
            break;
        default:
            throw std::runtime_error(err::format_error(
                node.location(), "Unknown binary arithmetic operator"));
            break;
    }
}

void Interpreter::visit(BinLogicOpNode& node)
{
    auto* left = node.left();
    auto* right = node.right();

    require_expr_node(left, node.location(), "BinLogicOpNode missing operand");
    require_expr_node(right, node.location(), "BinLogicOpNode missing operand");

    left->accept(*this);
    int64_t left_res = last_value_;
    switch (node.op()) {
        case bin_logic_op_type::greater:
            right->accept(*this);
            last_value_ = left_res > last_value_;
            break;
        case bin_logic_op_type::greater_equal:
            right->accept(*this);
            last_value_ = left_res >= last_value_;
            break;
        case bin_logic_op_type::less:
            right->accept(*this);
            last_value_ = left_res < last_value_;
            break;
        case bin_logic_op_type::less_equal:
            right->accept(*this);
            last_value_ = left_res <= last_value_;
            break;
        case bin_logic_op_type::equal:
            right->accept(*this);
            last_value_ = left_res == last_value_;
            break;
        case bin_logic_op_type::not_equal:
            right->accept(*this);
            last_value_ = left_res != last_value_;
            break;
        case bin_logic_op_type::logical_and:
            if (left_res == 0) {
                last_value_ = 0;
                break;
            }
            right->accept(*this);
            last_value_ = left_res && last_value_;
            break;
        case bin_logic_op_type::logical_or:
            if (left_res != 0) {
                last_value_ = 1;
                break;
            }
            right->accept(*this);
            last_value_ = left_res || last_value_;
            break;
        case bin_logic_op_type::bitwise_xor:
            right->accept(*this);
            last_value_ = left_res ^ last_value_;
            break;
        default:
            throw std::runtime_error(err::format_error(
                node.location(), "Unknown binary logical operator"));
            break;
    }
}

void Interpreter::visit(ValueNode& node)
{
    last_value_ = node.value();
}

void Interpreter::visit(UnOpNode& node)
{
    auto* operand = node.operand();
    require_expr_node(operand, node.location(), "UnOpNode missing operand");

    operand->accept(*this);
    switch (node.op()) {
        case unop_node_type::pos:
            break;
        case unop_node_type::neg:
            if (last_value_ == std::numeric_limits<int64_t>::min()) {
                throw std::runtime_error(err::format_error(
                    node.location(), "Integer overflow in unary minus"));
            }
            last_value_ = -last_value_;
            break;
        case unop_node_type::logical_not:
            last_value_ = !last_value_;
            break;
    }
}

void Interpreter::visit(AssignNode& node)
{
    auto* lhs = node.lhs();

    require_expr_node(lhs, node.location(), "AssignNode's lhs is missing");

    bool is_var = lhs->node_type() == base_node_type::var;
    if (!is_var) {
        throw std::runtime_error(
            err::format_error(node.location(), "AssignNode lhs must be var"));
    }
    const VarNode* var = static_cast<VarNode*>(lhs);

    auto* operand = node.rhs();
    require_expr_node(operand, node.location(), "AssignNode missing operand");

    operand->accept(*this);
    table_.assign_or_create(var->name(), last_value_);
}

void Interpreter::visit(VarNode& node)
{
    last_value_ = table_.lookup(node.name(), node.location());
}

void Interpreter::visit(IfNode& node)
{
    auto* cond = node.condition();
    require_expr_node(cond, node.location(), "Missing condition");
    auto* then_branch = node.then_branch();
    auto* else_branch = node.else_branch();

    detail::ScopeGuard scope_guard(table_, node.location());
    validate_evaluable_node(
        *cond, "Invalid condition", evaluable_context::condition);
    cond->accept(*this);
    if (last_value_) {
        accept_stmt_if_present(then_branch, *this);
    } else {
        accept_stmt_if_present(else_branch, *this);
    }
}

void Interpreter::visit(WhileNode& node)
{
    auto* cond = node.condition();
    auto* body = node.body();

    require_expr_node(cond, node.location(), "Missing condition");

    detail::ScopeGuard scope_guard(table_, node.location());
    const auto cond_var_name = validate_evaluable_node(
        *cond, "Invalid condition", evaluable_context::condition);
    evaluate_loop_condition(*cond, cond_var_name, true);

    while (last_value_) {
        accept_stmt_if_present(body, *this);
        evaluate_loop_condition(*cond, cond_var_name, false);
    }
}

void Interpreter::visit(ForNode& node)
{
    auto* init = node.get_init();
    auto* cond = node.get_cond();
    auto* step = node.get_step();
    auto* body = node.get_body();

    require_expr_node(cond, node.location(), "Missing condition");

    detail::ScopeGuard scope_guard(table_, node.location());

    accept_stmt_if_present(init, *this);

    const auto cond_var_name = validate_evaluable_node(
        *cond, "Invalid condition", evaluable_context::condition);
    evaluate_loop_condition(*cond, cond_var_name, true);

    while (last_value_) {
        accept_stmt_if_present(body, *this);
        accept_stmt_if_present(step, *this);
        evaluate_loop_condition(*cond, cond_var_name, false);
    }
}

void Interpreter::visit(InputNode& node)
{
    last_value_ = read_input_int64_or_throw(node.location());
}

void Interpreter::visit(ExprNode& node)
{
    auto* expr = node.expr();
    require_expr_node(expr, node.location(), "Expression is not valid");
    expr->accept(*this);
}

void Interpreter::visit(PrintNode& node)
{
    auto* expr = node.expr();
    require_expr_node(expr, node.location(), "Missing expression for printing");

    validate_evaluable_node(*expr, "Invalid print expression");
    expr->accept(*this);
    std::cout << last_value_ << std::endl;
}

void Interpreter::visit(ScopeNode& node)
{
    const bool need_scope = node.parent() != nullptr;
    if (need_scope) {
        detail::ScopeGuard scope_guard(table_, node.location());
        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
        return;
    }

    for (const auto& stmt : node.statements()) {
        stmt->accept(*this);
    }
}

void Interpreter::visit(VarDeclNode& node)
{
    const auto& init = node.init_expr();
    if (has_expr_node(init)) {
        init->accept(*this);
    } else {
        last_value_ = 0;
    }
    table_.declare_in_cur_scope(node.name(), last_value_, node.location());
}

void Interpreter::visit(ErrorNode& node)
{
    throw std::runtime_error(
        err::format_error(node.location(), "Cannot execute error node"));
}

void Interpreter::visit(EmptyNode&)
{
}

bool Interpreter::add_overflow(int64_t lhs, int64_t rhs, int64_t& out)
{
    constexpr int64_t kMax = std::numeric_limits<int64_t>::max();
    constexpr int64_t kMin = std::numeric_limits<int64_t>::min();
    if ((rhs > 0 && lhs > kMax - rhs) || (rhs < 0 && lhs < kMin - rhs)) {
        return true;
    }
    out = lhs + rhs;
    return false;
}

bool Interpreter::sub_overflow(int64_t lhs, int64_t rhs, int64_t& out)
{
    constexpr int64_t kMax = std::numeric_limits<int64_t>::max();
    constexpr int64_t kMin = std::numeric_limits<int64_t>::min();
    if ((rhs < 0 && lhs > kMax + rhs) || (rhs > 0 && lhs < kMin + rhs)) {
        return true;
    }
    out = lhs - rhs;
    return false;
}

bool Interpreter::mul_overflow(int64_t lhs, int64_t rhs, int64_t& out)
{
    constexpr int64_t kMax = std::numeric_limits<int64_t>::max();
    constexpr int64_t kMin = std::numeric_limits<int64_t>::min();

    if (lhs == 0 || rhs == 0) {
        out = 0;
        return false;
    }
    if ((lhs == -1 && rhs == kMin) || (rhs == -1 && lhs == kMin)) {
        return true;
    }

    if (lhs > 0) {
        if (rhs > 0) {
            if (lhs > kMax / rhs) {
                return true;
            }
        } else {
            if (rhs < kMin / lhs) {
                return true;
            }
        }
    } else {
        if (rhs > 0) {
            if (lhs < kMin / rhs) {
                return true;
            }
        } else {
            if (lhs < kMax / rhs) {
                return true;
            }
        }
    }

    out = lhs * rhs;
    return false;
}

void Interpreter::evaluate_loop_condition(
    BaseNode& condition,
    const std::optional<std::string>& tracked_var_name,
    bool initialize_tracked_var)
{
    if (tracked_var_name) {
        if (initialize_tracked_var) {
            condition.accept(*this);
        }
        last_value_ = table_.lookup(*tracked_var_name, condition.location());
        return;
    }

    condition.accept(*this);
}

std::optional<std::string> Interpreter::validate_evaluable_node(
    const BaseNode& node,
    const char* error_msg,
    evaluable_context context)
{
    if (node.node_type() == base_node_type::assign) {

        const auto* assign = dynamic_cast<const AssignNode*>(&node);
        if (!assign || is_missing_or_empty_expr_node(assign->lhs())) {
            throw std::runtime_error(err::format_error(
                node.location(), "AssignNode condition missing lhs"));
        }
        if (assign->lhs()->node_type() != base_node_type::var) {
            throw std::runtime_error(err::format_error(
                node.location(), "AssignNode condition lhs must be var"));
        }
        const auto* var = static_cast<const VarNode*>(assign->lhs());
        return var->name();
    }

    if (node.node_type() == base_node_type::var_decl) {
        if (context != evaluable_context::condition) {
            throw std::runtime_error(
                err::format_error(node.location(), error_msg));
        }

        const auto* decl = dynamic_cast<const VarDeclNode*>(&node);
        if (!decl) {
            throw std::runtime_error(err::format_error(
                node.location(), "Invalid VarDeclNode condition"));
        }
        return decl->name();
    }

    switch (node.node_type()) {
        case base_node_type::scope:
        case base_node_type::while_node:
        case base_node_type::print:
        case base_node_type::if_node:
        case base_node_type::for_node:
        case base_node_type::err:
        case base_node_type::empty:
            throw std::runtime_error(
                err::format_error(node.location(), error_msg));
        case base_node_type::base:
            throw std::runtime_error(err::format_error(
                node.location(), "you cannot use abstract class"));
        case base_node_type::assign:
        case base_node_type::var_decl:
        case base_node_type::bin_arith_op:
        case base_node_type::unop:
        case base_node_type::bin_logic_op:
        case base_node_type::value:
        case base_node_type::var:
        case base_node_type::input:
        case base_node_type::expr:
            return std::nullopt;
    }

    return std::nullopt;
}

} // namespace ast
