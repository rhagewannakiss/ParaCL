#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>

namespace ast {

struct Visitor;

enum class base_node_type {
    base,
    bin_arith_op,
    bin_logic_op,
    unop,
    scope,
    value,
    print,
    assign,
    var,
    expr,
    if_node,
    while_node,
    input,
};

class BaseNode
{
public:
    using NodePtr = std::unique_ptr<BaseNode>;

private:
    base_node_type node_type_ = base_node_type::base;
    BaseNode* parent_ = nullptr;
    std::deque<NodePtr> children_{};

protected:
    void set_child_parent(BaseNode* child)
    {
        if (child) {
            child->set_parent(this);
        }
    }

public:
    explicit BaseNode(base_node_type node_type) : 
                node_type_(node_type) {}
    virtual ~BaseNode() = default;

    base_node_type node_type() const  { return node_type_; }
    BaseNode*      parent() const     { return parent_; }
    void set_parent(BaseNode* parent) { parent_ = parent; }

    void add_child(NodePtr child)
    {
        set_child_parent(child.get());
        children_.push_back(std::move(child));
    }

    void add_child_front(NodePtr child)
    {
        set_child_parent(child.get());
        children_.push_front(std::move(child));
    }

    const std::deque<NodePtr>& children() const { return children_; }
    std::deque<NodePtr>& children() { return children_; }

    virtual void accept(Visitor& v) = 0;
};

class ValueNode : public BaseNode
{
    int value_;

public:
    explicit ValueNode(int value) 
             : BaseNode(base_node_type::value), value_(value) {}
    int value() const { return value_; }
    void accept(Visitor& v) override;
};

enum class unop_node_type {
    neg,
    pos,
    logical_not,
};

class UnOpNode : public BaseNode
{
    unop_node_type op_;

public:
    explicit UnOpNode(unop_node_type op, 
                      NodePtr operand = nullptr)
        : BaseNode(base_node_type::unop), op_(op)
    {
        if (operand) {
            set_operand(std::move(operand));
        }
    }

    void set_operand(NodePtr operand)
    {
        add_child(std::move(operand));
    }

    unop_node_type op() const { return op_; }

    const BaseNode* operand() const
    {
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
};

class PrintNode : public BaseNode
{
public:
    PrintNode() : BaseNode(base_node_type::print) {}

    explicit PrintNode(NodePtr expr)
        : BaseNode(base_node_type::print)
    {
        if (expr) {
            set_expr(std::move(expr));
        }
    }

    void set_expr(NodePtr expr)
    {
        add_child(std::move(expr));
    }

    const BaseNode* expr() const
    {
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
};

class AssignNode : public BaseNode
{
public:
    AssignNode() : BaseNode(base_node_type::assign) {}

    AssignNode(NodePtr lhs, NodePtr rhs)
        : BaseNode(base_node_type::assign)
    {
        if (lhs) {
            set_lhs(std::move(lhs));
        }
        if (rhs) {
            set_rhs(std::move(rhs));
        }
    }

    void set_lhs(NodePtr lhs)
    {
        add_child(std::move(lhs));
    }

    void set_rhs(NodePtr rhs)
    {
        add_child(std::move(rhs));
    }

    const BaseNode* lhs() const
    {
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* rhs() const
    {
        if (children().size() > 1) {
            return children()[1].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
};

class VarNode : public BaseNode
{
    std::string name_;

public:
    explicit VarNode(std::string name)
        : BaseNode(base_node_type::var), 
          name_(std::move(name)) {}

    const std::string& name() const { return name_; }
    void accept(Visitor& v) override;
};

class IfNode : public BaseNode
{
public:
    IfNode() : BaseNode(base_node_type::if_node) {}

    IfNode(NodePtr condition, 
           NodePtr then_branch, 
           NodePtr else_branch = nullptr)
        : BaseNode(base_node_type::if_node)
    {
        if (condition) {
            set_condition(std::move(condition));
        }
        if (then_branch) {
            set_then(std::move(then_branch));
        }
        if (else_branch) {
            set_else(std::move(else_branch));
        }
    }

    void set_condition(NodePtr cond)   { add_child(std::move(cond)); }
    void set_then(NodePtr then_branch) { add_child(std::move(then_branch)); }
    void set_else(NodePtr else_branch) { add_child(std::move(else_branch)); }

    const BaseNode* condition() const
    {
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* then_branch() const
    {
        if (children().size() > 1) {
            return children()[1].get();
        }
        return nullptr;
    }

    const BaseNode* else_branch() const
    {
        if (children().size() > 2) {
            return children()[2].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
};

class WhileNode : public BaseNode
{
public:
    WhileNode() : BaseNode(base_node_type::while_node) {}

    WhileNode(NodePtr condition, NodePtr body)
        : BaseNode(base_node_type::while_node)
    {
        if (condition) {
            set_condition(std::move(condition));
        }
        if (body) {
            set_body(std::move(body));
        }
    }

    void set_condition(NodePtr cond) { add_child(std::move(cond)); }
    void set_body(NodePtr body)      { add_child(std::move(body)); }

    const BaseNode* condition() const
    {
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* body() const
    {
        if (children().size() > 1) {
            return children()[1].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
};

class InputNode : public BaseNode
{
public:
    InputNode() : BaseNode(base_node_type::input) {}

    explicit InputNode(NodePtr lhs)
        : BaseNode(base_node_type::input)
    {
        if (lhs) {
            set_lhs(std::move(lhs));
        }
    }

    void set_lhs(NodePtr lhs)
    {
        add_child(std::move(lhs));
    }

    const BaseNode* lhs() const
    {
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
};

class ExprNode : public BaseNode
{
public:
    ExprNode() : BaseNode(base_node_type::expr) {}

    explicit ExprNode(NodePtr expr)
        : BaseNode(base_node_type::expr)
    {
        if (expr) {
            set_expr(std::move(expr));
        }
    }

    void set_expr(NodePtr expr)
    {
        add_child(std::move(expr));
    }

    const BaseNode* expr() const
    {
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
};

enum class bin_arith_op_type {
    add,
    sub,
    mul,
    div,
    pow,
};

class BinArithOpNode : public BaseNode
{
    bin_arith_op_type op_;

public:
    explicit BinArithOpNode(bin_arith_op_type op, 
                            NodePtr left = nullptr, 
                            NodePtr right = nullptr)
        : BaseNode(base_node_type::bin_arith_op), op_(op)
    {
        if (left) {
            set_left(std::move(left));
        }
        if (right) {
            set_right(std::move(right));
        }
    }

    void set_left(NodePtr child)  { add_child_front(std::move(child)); }
    void set_right(NodePtr child) { add_child(std::move(child)); }

    bin_arith_op_type op() const { return op_; }
    const BaseNode* left() const 
    { 
        if (children().size() > 0) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* right() const 
    { 
        if (children().size() > 1) {
            return children()[1].get();
        }
        return nullptr;
    }
    void accept(Visitor& v) override;
};

enum class bin_logic_op_type {
    greater,
    less,
    greater_equal,
    less_equal,
    equal,
    not_equal,
    logical_and,
    logical_or,
};

class BinLogicOpNode : public BaseNode
{
    bin_logic_op_type op_;

public:
    explicit BinLogicOpNode(bin_logic_op_type op, 
                            NodePtr left = nullptr, 
                            NodePtr right = nullptr)
        : BaseNode(base_node_type::bin_logic_op), op_(op)
    {
        if (left) {
            set_left(std::move(left));
        }
        if (right) {
            set_right(std::move(right));
        }
    }

    void set_left(NodePtr child)  { add_child_front(std::move(child)); }
    void set_right(NodePtr child) { add_child(std::move(child)); }

    bin_logic_op_type op() const { return op_; }
    const BaseNode* left() const 
    { 
        if (children().size() > 0) 
            return children()[0].get();
        
        return nullptr;
    }

    const BaseNode* right() const 
    { 
        if (children().size() > 1) 
            return children()[1].get();
        
        return nullptr;
    }
    void accept(Visitor& v) override;
};

class ScopeNode : public BaseNode
{
public:
    ScopeNode() : BaseNode(base_node_type::scope) {}

    explicit ScopeNode(std::vector<NodePtr> statements)
        : BaseNode(base_node_type::scope)
    {
        for (auto& stmt : statements) {
            add_child(std::move(stmt));
        }
    }

    void add_statement(NodePtr statement) {
        add_child(std::move(statement));
    }

    const std::deque<NodePtr>& statements() const { return children(); }

    void accept(Visitor& v) override;
};

class AST
{
public:
    using NodePtr = BaseNode::NodePtr;

private:
    NodePtr root_{};

public:
    AST() = default;

    explicit AST(NodePtr root)
        : root_(std::move(root)) {}

    AST(const AST&) = delete;
    AST& operator=(const AST&) = delete;

    AST(AST&&) noexcept = default;
    AST& operator=(AST&&) noexcept = default;

    const BaseNode* root() const { return root_.get(); }
    BaseNode* root() { return root_.get(); }

    void set_root(NodePtr root) { root_ = std::move(root); }
};

} // namespace ast
