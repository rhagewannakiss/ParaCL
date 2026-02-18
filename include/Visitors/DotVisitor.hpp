#pragma once

#include <ostream> 
#include <sstream>
#include <string>
#include <unordered_map>

#include "Visitors/Visitor.hpp"

namespace ast {
//TODO: добавить константный Visitor и переписать DotVisitor на DotConstVisitor

class DotVisitor : public Visitor
{
public:
    explicit DotVisitor(std::ostream& out) : out_(out) {}

    void visit(BinArithOpNode& node) override
    {
        emit_node(node, op_label(node.op()));
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(BinLogicOpNode& node) override
    {
        emit_node(node, op_label(node.op()));
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(ValueNode& node) override
    {
        emit_node(node, std::to_string(node.value()));
    }

    void visit(UnOpNode& node) override
    {
        emit_node(node, unop_label(node.op()));
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(AssignNode& node) override
    {
        emit_node(node, "=");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(VarNode& node) override
    {
        emit_node(node, node.name());
    }

    void visit(IfNode& node) override
    {
        emit_node(node, "if");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(WhileNode& node) override
    {
        emit_node(node, "while");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }
    void visit(ForNode& node) override
    {
        emit_node(node, "for");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(InputNode& node) override
    {
        emit_node(node, "input ?");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(ExprNode& node) override
    {
        emit_node(node, "expr");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(PrintNode& node) override
    {
        emit_node(node, "print");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(ScopeNode& node) override
    {
        emit_node(node, "scope");
        emit_edges(node);
        for (const auto& child : node.children()) {
            child->accept(*this);
        }
    }

    void visit(VarDeclNode& node) override
    {
        emit_node(node, "var_decl " + node.name());
        emit_edges(node);
        const auto& init_expr = node.init_expr();
        if(init_expr != nullptr)
            init_expr->accept(*this);
    }

    void create_dot(ast::AST& ast) 
    {
        begin_graph();
        if(ast.root()) {
            ast.root()->accept(*this);
        }
        end_graph();
    }

    void begin_graph() { out_ << "digraph AST {\n"; }
    void end_graph()   { out_ << "}\n"; }

private:
    std::ostream& out_;
    std::unordered_map<const BaseNode*, std::string> ids_;
    size_t next_id_ = 0;

    std::string id_for(const BaseNode* node)
    {
        auto it = ids_.find(node);
        if (it != ids_.end()) {
            return it->second;
        }
        std::string id = "n" + std::to_string(next_id_++);
        ids_.emplace(node, id);
        return id;
    }

    template <typename NodeT>
    void emit_node(NodeT& node, const std::string& payload)
    {
        out_ << "  " << id_for(&node) << " [shape=box,label=\""
             << node_type_name(node.node_type()) << "\\n"
             << payload << "\\n"
             << addr_of(node) << "\"]\n";
    }

    void emit_edges(BaseNode& node)
    {
        const auto& kids = node.children();
        for (const auto& child : kids) {
            out_ << "  " << id_for(&node) << " -> " << id_for(child.get()) << "\n";
        }
    }

    std::string node_type_name(base_node_type type)
    {
        switch (type) {
            case base_node_type::bin_arith_op: return "bin_arith_op";
            case base_node_type::bin_logic_op: return "bin_logic_op";
            case base_node_type::unop:         return "unop";
            case base_node_type::scope:        return "scope";
            case base_node_type::value:        return "value";
            case base_node_type::print:        return "print";
            case base_node_type::assign:       return "assign";
            case base_node_type::var:          return "var";
            case base_node_type::expr:         return "expr";
            case base_node_type::if_node:      return "if";
            case base_node_type::while_node:   return "while";
            case base_node_type::input:        return "input";
            case base_node_type::base:         return "base";
            case base_node_type::var_decl:     return "var_decl";
            case base_node_type::for_node:     return "for";
        }
        return "unknown";
    }

    template <typename NodeT>
    std::string addr_of(const NodeT& node)
    {
        std::ostringstream oss;
        oss << static_cast<const void*>(&node);
        return oss.str();
    }

    std::string op_label(bin_arith_op_type op)
    {
        switch (op) {
            case bin_arith_op_type::add: return "+";
            case bin_arith_op_type::sub: return "-";
            case bin_arith_op_type::mul: return "*";
            case bin_arith_op_type::div: return "/";
            case bin_arith_op_type::mod: return "%";
        }
        return "?";
    }

    std::string op_label(bin_logic_op_type op)
    {
        switch (op) {
            case bin_logic_op_type::greater:       return ">";
            case bin_logic_op_type::less:          return "<";
            case bin_logic_op_type::greater_equal: return ">=";
            case bin_logic_op_type::less_equal:    return "<=";
            case bin_logic_op_type::equal:         return "==";
            case bin_logic_op_type::not_equal:     return "!=";
            case bin_logic_op_type::logical_and:   return "&&";
            case bin_logic_op_type::logical_or:    return "||";
            case bin_logic_op_type::bitwise_xor:   return "^";
        }
        return "?";
    }

    std::string unop_label(unop_node_type op)
    {
        switch (op) {
            case unop_node_type::pos:         return "+";
            case unop_node_type::neg:         return "-";
            case unop_node_type::logical_not: return "!";
        }
        return "?";
    }
};
} // namespace ast
