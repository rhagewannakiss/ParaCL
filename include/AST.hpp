#pragma once

#include <cassert>
#include <deque>
#include <memory>
#include <string>
#include <vector>
#ifdef NDEBUG
#include <stdexcept>
#endif
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

inline void ensure_child_free(bool already_set, 
                              const char* msg) 
{
#ifndef NDEBUG
    if (already_set) {
        std::fputs(msg, stderr);
        std::fputs("\n", stderr);
        assert(!already_set);
    }    
#else 
    if (already_set) {
        throw std::logic_error(msg);
    }
#endif
}

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

    BaseNode(const BaseNode& other) : 
        node_type_(other.node_type_) {}
    
    BaseNode& operator=(const BaseNode& other)
    {
        if (this == &other) return *this;
        node_type_ = other.node_type_;
        parent_ = nullptr;
        children_.clear();
        return *this;
    }

    BaseNode(BaseNode&& other) noexcept
        : node_type_(other.node_type_), 
        parent_(nullptr), 
        children_(std::move(other.children_))
    {
        for (auto& child : children_) {
            set_child_parent(child.get());
        }
    }

    BaseNode& operator=(BaseNode&& other) noexcept
    {
        if (this == &other) return *this;
        node_type_ = other.node_type_;
        parent_ = nullptr;
        children_ = std::move(other.children_);
        for (auto& child : children_) {
            set_child_parent(child.get());
        }
        return *this;
    }


public:
    explicit BaseNode(base_node_type node_type) : 
                node_type_(node_type) {}
    virtual ~BaseNode() = default;

    base_node_type node_type() const            { return node_type_; }
    BaseNode*      parent() const               { return parent_; }
    void           set_parent(BaseNode* parent) { parent_ = parent; }

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
    virtual NodePtr clone() const = 0;
};

class ValueNode : public BaseNode
{
    int value_;

public:
    explicit ValueNode(int value) : 
        BaseNode(base_node_type::value), 
        value_(value) {}
    
    ValueNode(const ValueNode& other) : 
        BaseNode(other), value_(other.value_) {}

    ValueNode& operator=(const ValueNode& other) {
        if (this == &other) return *this;
        BaseNode::operator=(other);
        value_ = other.value_;
        return *this;
    }

    ValueNode(ValueNode&& other) noexcept = default;
    ValueNode& operator=(ValueNode&& other) noexcept = default;

    int value() const { return value_; }
    void accept(Visitor& v) override;

    NodePtr clone() const override {
        return std::make_unique<ValueNode>(*this);
    }
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

    UnOpNode(const UnOpNode& other) :
        BaseNode(other), op_(other.op_) 
    {
        if(other.operand()) {
            set_operand(other.operand()->clone());
        }
    }

    UnOpNode& operator=(const UnOpNode& other) {
        if (this == &other) return *this;
        BaseNode::operator=(other);
        children().clear();
        if (other.operand()) {
            set_operand(other.operand()->clone());
        }
        op_ = other.op_;
        return *this;
    }

    UnOpNode(UnOpNode&& other) noexcept = default;
    UnOpNode& operator=(UnOpNode&& other) noexcept = default;

    void set_operand(NodePtr operand)
    {
        ensure_child_free(children().size() != 0, 
                          "operand is already set");
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
    
    NodePtr clone() const override {
        return std::make_unique<UnOpNode>(*this);
    }
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

    PrintNode(const PrintNode& other) 
        : BaseNode(other) 
    {
        if (other.expr()) {
            set_expr(other.expr()->clone());
        }
    }

    PrintNode& operator=(const PrintNode& other) 
    {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        children().clear();
        if (other.expr()) {
            set_expr(other.expr()->clone());
        }
        return *this;
    }

    PrintNode(PrintNode&& other) noexcept = default;
    PrintNode& operator=(PrintNode&& other) noexcept = default;

    void set_expr(NodePtr expr)
    {
        ensure_child_free(children().size() != 0, 
                          "expr is already set");
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

    NodePtr clone() const override {
        return std::make_unique<PrintNode>(*this);
    }
};

class AssignNode : public BaseNode
{
    bool is_lhs_set = false;
    bool is_rhs_set = false;
public:
    AssignNode() : BaseNode(base_node_type::assign) {}

    AssignNode(NodePtr lhs = nullptr, 
               NodePtr rhs = nullptr)
        : BaseNode(base_node_type::assign)
    {
        if (lhs) {
            set_lhs(std::move(lhs));
        } else {
            is_lhs_set = false;
        }
        if (rhs) {
            set_rhs(std::move(rhs));
        } else {
            is_rhs_set = false;
        }
    }

    AssignNode(const AssignNode& other) 
        : BaseNode(other) 
    {
        if (other.lhs()) {
            set_lhs(other.lhs()->clone());
        }
        if (other.rhs()) {
            set_rhs(other.rhs()->clone());
        }
    }

    AssignNode& operator=(const AssignNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        is_lhs_set = false;
        is_rhs_set = false;
        children().clear();
        if (other.lhs()) {
            set_lhs(other.lhs()->clone());
        }
        if (other.rhs()) {
            set_rhs(other.rhs()->clone());
        }
        return *this;
    }

    AssignNode(AssignNode&& other) noexcept = default;
    AssignNode& operator=(AssignNode&& other) noexcept = default;

    void set_lhs(NodePtr lhs)
    {
        ensure_child_free(is_lhs_set, "lhs is already set");
        add_child(std::move(lhs));
        is_lhs_set = true;
    }

    void set_rhs(NodePtr rhs)
    {
        ensure_child_free(is_rhs_set, "rhs is already set");
        add_child(std::move(rhs));
        is_rhs_set = true;
    }

    const BaseNode* lhs() const
    {
        if (children().size() > 0 && is_lhs_set) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* rhs() const
    {
        if (children().size() > 1 && is_rhs_set) {
            return children()[1].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
    NodePtr clone() const override {
        return std::make_unique<AssignNode>(*this);
    }
};

class VarNode : public BaseNode
{
    std::string name_;

public:
    explicit VarNode(std::string name)
        : BaseNode(base_node_type::var), 
          name_(std::move(name)) {}

    VarNode (const VarNode& other)
        : BaseNode(other), name_(other.name_) {}

    VarNode& operator=(const VarNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        name_ = other.name_;
        return *this;
    }

    VarNode(VarNode&& other) noexcept = default;
    VarNode& operator=(VarNode&& other) noexcept = default;

    const std::string& name() const { return name_; }
    void accept(Visitor& v) override;
    NodePtr clone() const override {
        return std::make_unique<VarNode>(*this);
    }
};

class IfNode : public BaseNode
{
    bool is_condition_set = false;
    bool is_then_set = false;
    bool is_else_set = false;
public:
    IfNode() : BaseNode(base_node_type::if_node) {}

    IfNode(NodePtr condition   = nullptr, 
           NodePtr then_branch = nullptr, 
           NodePtr else_branch = nullptr)
        : BaseNode(base_node_type::if_node)
    {
        if (condition) {
            set_condition(std::move(condition));
        } else {
            is_condition_set = false;
        }
        if (then_branch) {
            set_then(std::move(then_branch));
        } else {
            is_then_set = false;
        }
        if (else_branch) {
            set_else(std::move(else_branch));
        } else {
            is_else_set = false;
        }
    }

    IfNode(const IfNode& other) 
        : BaseNode(other)
    {
        if(other.condition()) {
            set_condition(other.condition()->clone());
        }
        if(other.then_branch()) {
            set_then(other.then_branch()->clone());
        }
        if(other.else_branch()) {
            set_else(other.else_branch()->clone());
        }
    }

    IfNode& operator=(const IfNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        is_condition_set = false;
        is_then_set      = false;
        is_else_set      = false;

        children().clear();
        if(other.condition()) {
            set_condition(other.condition()->clone());
        }
        if(other.then_branch()) {
            set_then(other.then_branch()->clone());
        }
        if(other.else_branch()) {
            set_else(other.else_branch()->clone());
        }
        return *this;
    }

    IfNode(IfNode&& other) noexcept = default;
    IfNode& operator=(IfNode&& other) noexcept = default;

    void set_condition(NodePtr cond)   
    {
        ensure_child_free(is_condition_set, "condition is already set");
        add_child(std::move(cond));
        is_condition_set = true;
    }
    void set_then(NodePtr then_branch) 
    {
        ensure_child_free(is_then_set, "then is already set");
        add_child(std::move(then_branch));
        is_then_set = true;
    }
    void set_else(NodePtr else_branch) 
    {
        ensure_child_free(is_else_set, "else is already set");
        add_child(std::move(else_branch));
        is_else_set = true;
    }

    const BaseNode* condition() const
    {
        if (children().size() > 0 && is_condition_set) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* then_branch() const
    {
        if (children().size() > 1 && is_then_set) {
            return children()[1].get();
        }
        return nullptr;
    }

    const BaseNode* else_branch() const
    {
        if (children().size() > 2 && is_else_set) {
            return children()[2].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
    NodePtr clone() const override {
        return std::make_unique<IfNode>(*this);
    }
};

class WhileNode : public BaseNode
{
    bool is_condition_set = false;
    bool is_body_set = false;
public:
    WhileNode() : BaseNode(base_node_type::while_node) {}

    WhileNode(NodePtr condition = nullptr, 
              NodePtr body      = nullptr)
        : BaseNode(base_node_type::while_node)
    {
        if (condition) {
            set_condition(std::move(condition));
        } else {
            is_condition_set = false;
        }
        if (body) {
            set_body(std::move(body));
        } else {
            is_body_set = false;
        }
    }

    WhileNode(const WhileNode& other) 
        : BaseNode(other)
    {
        if(other.condition()) {
            set_condition(other.condition()->clone());
        }
        if(other.body()) {
            set_body(other.body()->clone());
        }
    }
            
    WhileNode& operator=(const WhileNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        is_condition_set = false;
        is_body_set      = false;

        children().clear();
        if(other.condition()) {
            set_condition(other.condition()->clone());
        }
        if(other.body()) {
            set_body(other.body()->clone());
        }
        return *this;
    }

    WhileNode(WhileNode&& other) noexcept = default;
    WhileNode& operator=(WhileNode&& other) noexcept = default;

    void set_condition(NodePtr cond) 
    {
        ensure_child_free(is_condition_set, "condition is already set");
        add_child(std::move(cond));
        is_condition_set = true;
    }
    void set_body(NodePtr body)
    {
        ensure_child_free(is_body_set, "body is already set");
        add_child(std::move(body));
        is_body_set = true;
    }

    const BaseNode* condition() const
    {
        if (children().size() > 0 && is_condition_set) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* body() const
    {
        if (children().size() > 1 && is_body_set) {
            return children()[1].get();
        }
        return nullptr;
    }

    void accept(Visitor& v) override;
    NodePtr clone() const override {
        return std::make_unique<WhileNode>(*this);
    }
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

    InputNode(const InputNode& other) 
        : BaseNode(other) 
    {
        if (other.lhs()) {
            set_lhs(other.lhs()->clone());
        }
    }

    InputNode& operator=(const InputNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        children().clear();
        if (other.lhs()) {
            set_lhs(other.lhs()->clone());
        }
        return *this;
    }

    InputNode(InputNode&& other) noexcept = default;
    InputNode& operator=(InputNode&& other) noexcept = default;

    void set_lhs(NodePtr lhs)
    {
        ensure_child_free(children().size() != 0, "lhs is already set");
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
    NodePtr clone() const override {
        return std::make_unique<InputNode>(*this);
    }
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

    ExprNode(const ExprNode& other) 
        : BaseNode(other) 
    {
        if (other.expr()) {
            set_expr(other.expr()->clone());
        }
    }

    ExprNode& operator=(const ExprNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        children().clear();
        if (other.expr()) {
            set_expr(other.expr()->clone());
        }
        return *this;
    }

    ExprNode(ExprNode&& other) noexcept = default;
    ExprNode& operator=(ExprNode&& other) noexcept = default;

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
    NodePtr clone() const override {
        return std::make_unique<ExprNode>(*this);
    }
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
    bool is_left_set  = false ;
    bool is_right_set = false;

public:
    explicit BinArithOpNode(bin_arith_op_type op, 
                            NodePtr left = nullptr, 
                            NodePtr right = nullptr)
        : BaseNode(base_node_type::bin_arith_op), op_(op)
    {
        if (left) {
            set_left(std::move(left));
        } else {
            is_left_set = false;
        }
        if (right) {
            set_right(std::move(right));
        } else {
            is_right_set = false;
        }
    }

    BinArithOpNode(const BinArithOpNode& other) 
        : BaseNode(other), op_(other.op_) 
    {
        if (other.left()) {
            set_left(other.left()->clone());
        }
        if (other.right()) {
            set_right(other.right()->clone());
        }
    }

    BinArithOpNode& operator=(const BinArithOpNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        op_ = other.op_;
        is_left_set = false;
        is_right_set = false;
        children().clear();
        if (other.left()) {
            set_left(other.left()->clone());
        }
        if (other.right()) {
            set_right(other.right()->clone());
        }
        return *this;
    }

    BinArithOpNode(BinArithOpNode&& other) noexcept = default;
    BinArithOpNode& operator=(BinArithOpNode&& other) noexcept = default;

    void set_left(NodePtr child)  
    {
        ensure_child_free(is_left_set, "left is already set");
        add_child(std::move(child));
        is_left_set = true;
    }
    
    void set_right(NodePtr child) 
    {
        ensure_child_free(is_right_set, "right is already set");
        add_child(std::move(child));
        is_right_set = true;
    }

    bin_arith_op_type op() const { return op_; }
    const BaseNode* left() const 
    { 
        if (children().size() > 0 && is_left_set) {
            return children()[0].get();
        }
        return nullptr;
    }

    const BaseNode* right() const 
    { 
        if (children().size() > 1 && is_right_set) {
            return children()[1].get();
        }
        return nullptr;
    }
    void accept(Visitor& v) override;
    NodePtr clone() const override {
        return std::make_unique<BinArithOpNode>(*this);
    }
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
    bool is_left_set  = false;  
    bool is_right_set = false; 

public:
    explicit BinLogicOpNode(bin_logic_op_type op, 
                            NodePtr left = nullptr, 
                            NodePtr right = nullptr)
        : BaseNode(base_node_type::bin_logic_op), op_(op)
    {
        if (left) {
            set_left(std::move(left));
        } else {
            is_left_set = false;
        }
        if (right) {
            set_right(std::move(right));
        } else {
            is_right_set = false;
        }
    }

    BinLogicOpNode(const BinLogicOpNode& other)
        : BaseNode(other), op_(other.op_)
    {
        if (other.left()) {
            set_left(other.left()->clone());
        }
        if (other.right()) {
            set_right(other.right()->clone());
        }
    }

    BinLogicOpNode& operator=(const BinLogicOpNode& other) {
        if(this == &other) return *this;
        BaseNode::operator=(other);
        op_ = other.op_;
        is_left_set = false;
        is_right_set = false;
        children().clear();
        if (other.left()) {
            set_left(other.left()->clone());
        }
        if (other.right()) {
            set_right(other.right()->clone());
        }
        return *this;
    }

    BinLogicOpNode(BinLogicOpNode&& other) noexcept = default;
    BinLogicOpNode& operator=(BinLogicOpNode&& other) noexcept = default;

    void set_left(NodePtr child)  
    {
        ensure_child_free(is_left_set, "left is already set");
        add_child(std::move(child));
        is_left_set = true;
    }
    void set_right(NodePtr child) 
    {
        ensure_child_free(is_right_set, "right is already set");
        add_child(std::move(child));
        is_right_set = true;
    }

    bin_logic_op_type op() const { return op_; }
    const BaseNode* left() const 
    { 
        if (children().size() > 0 && is_left_set) 
            return children()[0].get();
        
        return nullptr;
    }

    const BaseNode* right() const 
    { 
        if (children().size() > 1 && is_right_set) 
            return children()[1].get();
        
        return nullptr;
    }
    void accept(Visitor& v) override;
    NodePtr clone() const override {
        return std::make_unique<BinLogicOpNode>(*this);
    }
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

    ScopeNode(const ScopeNode& other) : BaseNode(other) {
        for (auto& child : other.children()) {
            add_child(child->clone());
        }
    }

    ScopeNode& operator=(const ScopeNode& other) {
        if (this == &other) return *this;
        BaseNode::operator=(other);
        children().clear();
        for (auto& child : other.children()) {
            add_child(child->clone());
        }
        return *this;
    }

    ScopeNode(ScopeNode&& other) noexcept = default;
    ScopeNode& operator=(ScopeNode&& other) noexcept = default;

    void add_statement(NodePtr statement) {
        add_child(std::move(statement));
    }

    const std::deque<NodePtr>& statements() const { return children(); }

    void accept(Visitor& v) override;
    NodePtr clone() const override {
        return std::make_unique<ScopeNode>(*this);
    }
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

    AST(const AST& other) 
        : AST(nullptr) 
    {
        if(other.root())
            root_ = other.root()->clone();
    }

    AST& operator=(const AST& other)
    {
        if(this == &other) return *this;
        if (other.root()) {
            root_ = other.root()->clone();
        } else { root_ = nullptr; }
        return *this;
    }

    AST(AST&&) noexcept = default;
    AST& operator=(AST&&) noexcept = default;

    const BaseNode* root() const { return root_.get(); }
    BaseNode* root() { return root_.get(); }

    void set_root(NodePtr root) { root_ = std::move(root); }
};

} // namespace ast
