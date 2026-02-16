#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <cassert>
#include <limits>

#include "AST/AST.hpp"
#include "AST/SourceRange.hpp"
#include "Visitors/Visitor.hpp"

namespace ast {

inline std::string make_runtime_error(const SourceRange& loc,
                                      const std::string& message)
{
    const std::string location = loc.make_string();
    if (location.empty()) {
        return "error: " + message;
    }
    return location + ": error: " + message;
}

class VarTable {
    using scope = std::unordered_map<std::string, int64_t>;
    std::vector<scope> scopes_;

public:
    VarTable() {scopes_.emplace_back();};
    
    
    void enter_scope() {
        scopes_.emplace_back();
    }

    void leave_scope(SourceRange loc = {}) {
        if (scopes_.size() <= 1) {
            throw std::runtime_error(
                make_runtime_error(loc, "Trying to leave from global scope")); 
        }
        scopes_.pop_back();
    }

    void declare_in_cur_scope(const std::string& name, 
                            int64_t value = 0,
                            SourceRange loc = {})
    {
        auto& cur = scopes_.back();
        const auto& iter = cur.find(name);
        if (iter != cur.end()) {
            throw std::runtime_error(
                    make_runtime_error(
                        loc,
                        "Variable " + name + " already declared"));
        } else {
            cur[name] = value;
        }
    }

    int64_t lookup(const std::string& name,
                    SourceRange loc = {}) {
        for(size_t i = scopes_.size(); i-- > 0 ;)
        {
            auto& cur = scopes_[i];
            const auto& iter = cur.find(name);
            if(iter != cur.end()) {
                return iter->second;  
            }
        }
        throw std::runtime_error(
                make_runtime_error(loc, "Undefined variable: " + name));
    }

    void assign_or_create(const std::string& name, int64_t value) {
        for (size_t i = scopes_.size(); i-- > 0 ;) {
            auto& cur = scopes_[i];
            const auto& iter = cur.find(name);
            if(iter != cur.end()) {
                iter->second = value;
                return;
            }
        }
        scopes_.back()[name] = value;
    }
};

class Interpreter : public Visitor {
    VarTable table_;
    int64_t last_value_;
public:
    explicit Interpreter() : 
        last_value_(0) {}

    void visit(BinArithOpNode& node) override
    {
        auto* left = node.left();
        auto* right = node.right();
        if (!left || !right) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "BinArithOpNode missing operand"));
        }
        left->accept(*this);
        int64_t left_res = last_value_;
        right->accept(*this);
        int64_t right_res = last_value_;
        switch (node.op()) {
            case bin_arith_op_type::add:
                last_value_ = left_res + right_res;
                break;
            case bin_arith_op_type::sub:
                last_value_ = left_res - right_res;
                break;
            case bin_arith_op_type::mul:
                last_value_ = left_res * right_res;
                break;
            case bin_arith_op_type::div:
                if(right_res == 0) {
                    throw std::runtime_error(
                            make_runtime_error(
                                node.location(),
                                "Division by zero"));
                }
                last_value_ = left_res / right_res;
                break;
            case ast::bin_arith_op_type::mod:
                if(right_res == 0) {
                    throw std::runtime_error(
                            make_runtime_error(
                                node.location(),
                                "Division by zero"));
                }
                last_value_ = left_res % right_res;
                break;
            default:
                throw std::runtime_error(
                        make_runtime_error(
                            node.location(),
                            "Unknown binary arithmetic operator"));
                break;
        }
    }

    void visit(BinLogicOpNode& node) override
    {
        auto* left = node.left();
        auto* right = node.right();

        if (!left || !right) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "BinLogicOpNode missing operand"));
        }
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
                if(left_res == 0) {
                    last_value_ = 0;
                    break;
                }
                right->accept(*this);
                last_value_ = left_res && last_value_;
                break;
            case bin_logic_op_type::logical_or:
                if(left_res != 0) {
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
                throw std::runtime_error(
                        make_runtime_error(
                            node.location(),
                            "Unknown binary logical operator"));
                break;
        }
    }

    void visit(ValueNode& node) override
    {
        last_value_ = node.value();
    }

    void visit(UnOpNode& node) override
    {
        auto* operand = node.operand();
        if (!operand) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "UnOpNode missing operand"));
        }
        operand->accept(*this);
        switch (node.op()) {
            case unop_node_type::pos:
                break;
            case unop_node_type::neg:
                last_value_ = last_value_ * (-1);
                break;
            case unop_node_type::logical_not:
                last_value_ = !last_value_;
                break;
        }
    }

    void visit(AssignNode& node) override
    {
        auto* lhs = node.lhs();

        if (lhs == nullptr) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "AssignNode's lhs is nullptr"));
        }
        bool is_var = lhs->node_type() == base_node_type::var;
        if(!is_var) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "AssignNode lhs must be var")); 
        }
        VarNode* var = static_cast<VarNode*>(lhs);
        
        auto* operand = node.rhs();
        if (!operand) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "AssignNode missing operand"));
        }
        
        operand->accept(*this);
        table_.assign_or_create(var->name(), last_value_);
    }

    void visit(VarNode& node) override 
    {
        last_value_ = table_.lookup(node.name(), node.location());
    }

    void visit(IfNode& node) override
    {
        auto* cond = node.condition();
        if (!cond) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "Missing condition"));
        }
        validate_evaluable_node(*cond, "Invalid condition");
        auto* then_branch = node.then_branch();
        auto* else_branch = node.else_branch();

        cond->accept(*this);
        if (last_value_) {
            if(then_branch == nullptr) {
                throw std::runtime_error(
                        make_runtime_error(
                            node.location(),
                            "Missing then branch"));
            }
            then_branch->accept(*this);
        } else if (else_branch) {
            else_branch->accept(*this);
        }     
    }

    void visit(WhileNode& node) override
    {
        auto* cond = node.condition();
        auto* body = node.body();
        
        if (!cond) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "Missing condition"));
        }
        if (!body) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "Missing while body"));
        }
        validate_evaluable_node(*cond, "Invalid condition");
        if(body->node_type() != base_node_type::scope) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "Invalid while body")); 
        }
        cond->accept(*this);
        while (last_value_) {
            body->accept(*this);
            cond->accept(*this);
        }
    }
    
    void visit(InputNode& node) override
    {
        auto* operand = node.lhs();
        if (!operand) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "InputNode missing operand"));
        }
        int64_t value = 0;
        if(operand->node_type() != ast::base_node_type::var) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "InputNode lhs must be var"));
        }
        VarNode* var = static_cast<VarNode*>(operand);
        
        if (!(std::cin >> value)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "Input error: expected integer"));
        }
        
        table_.assign_or_create(var->name(), value);
    }

    void visit(ExprNode& node) override
    {
        if(!(node.expr())) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "Expression is not valid"));
        }
        node.expr()->accept(*this);    
    }

    void visit(PrintNode& node) override
    {
        auto* expr = node.expr();
        if(!expr) {
            throw std::runtime_error(
                    make_runtime_error(
                        node.location(),
                        "Missing expression for printing"));
        }

        validate_evaluable_node(*expr, "Invalid print expression");
        expr->accept(*this);
        std::cout << last_value_ << std::endl;
    }

    void visit(ScopeNode& node) override
    {
        bool need_scope = node.parent() != nullptr;
        if(need_scope)
            table_.enter_scope();
        
        auto& stmts = node.statements();
        try {
            for(const auto& stmt : stmts) {
                stmt->accept(*this);
            }
        } catch ( ... ) {
            if(need_scope) table_.leave_scope(node.location());
            throw;
        }

        if(need_scope)
            table_.leave_scope(node.location());
    }

    void visit(VarDeclNode& node) override
    {
        const auto& init = node.init_expr();
        if(init) {
            init->accept(*this);
        } else {
            last_value_ = 0;
        }
        table_.declare_in_cur_scope(
                node.name(), 
                last_value_, 
                node.location()); 
    }

private:
    void validate_evaluable_node(
            const BaseNode& node, 
            const char* error_msg) const
    {
        switch (node.node_type()) {
            case base_node_type::scope:
            case base_node_type::assign:
            case base_node_type::while_node:
            case base_node_type::input:
            case base_node_type::var_decl:
            case base_node_type::print:
            case base_node_type::if_node:
                throw std::runtime_error(
                        make_runtime_error(node.location(), error_msg));
            case base_node_type::base:
                throw std::runtime_error(
                        make_runtime_error(
                            node.location(),
                            "you cannot use abstract class"));
            case base_node_type::bin_arith_op:
            case base_node_type::unop:
            case base_node_type::bin_logic_op:
            case base_node_type::value:
            case base_node_type::var:
            case base_node_type::expr:
                return;
        }
    }

};
} //namespace ast
