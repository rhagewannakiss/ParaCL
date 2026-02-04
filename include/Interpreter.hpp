#pragma once

#include <cmath>
#include <stdexcept>
#include <unordered_map>

#include "Visitor.hpp"

namespace ast {
class Interpreter : public Visitor {
    std::unordered_map<std::string, int64_t> vars_;
    int64_t last_value_;
public:
    //TODO: напистаь конструктор
    explicit Interpreter() 
    {
        last_value_ = 0;
    }

    void visit(BinArithOpNode& node) override
    {
        auto* left = node.left();
        auto* right = node.right();
        if (!left || !right) {//пока я нигде не ловлю это исключение и хз, что с ним делать
            throw std::runtime_error("BinArithOpNode missing operand");
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
                last_value_ = left_res / right_res;
                break;
            case bin_arith_op_type::pow:
                //TODO: можно написать свой целочисленный pow
                last_value_ = static_cast<int64_t>(
                    std::pow(left_res, right_res));
                break;
        }
    }
    void visit(BinLogicOpNode& node) override
    {
        
    }
    void visit(ValueNode& node) override
    {

    }
    void visit(UnOpNode& node) override
    {

    }
    void visit(AssignNode& node) override
    {

    }
    void visit(VarNode& node) override
    {

    }
    void visit(IfNode& node) override
    {

    }
    void visit(WhileNode& node) override
    {

    }
    void visit(InputNode& node) override
    {

    }
    void visit(ExprNode& node) override
    {

    }
    void visit(PrintNode& node) override
    {

    }
    void visit(ScopeNode& node) override
    {

    }

private:

};
} //namespace ast
