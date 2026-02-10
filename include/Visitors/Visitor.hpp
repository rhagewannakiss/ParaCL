#pragma once

#include "AST/AST.hpp"

namespace ast {

struct Visitor
{
    virtual ~Visitor() = default;

    virtual void visit(BinArithOpNode& node) = 0;
    virtual void visit(BinLogicOpNode& node) = 0;
    virtual void visit(ValueNode& node)      = 0;
    virtual void visit(UnOpNode& node)       = 0;
    virtual void visit(AssignNode& node)     = 0;
    virtual void visit(VarNode& node)        = 0;
    virtual void visit(IfNode& node)         = 0;
    virtual void visit(WhileNode& node)      = 0;
    virtual void visit(InputNode& node)      = 0;
    virtual void visit(ExprNode& node)       = 0;
    virtual void visit(PrintNode& node)      = 0;
    virtual void visit(ScopeNode& node)      = 0;
    virtual void visit(VarDeclNode& node)    = 0;
};

inline void BinArithOpNode::accept(Visitor& v) { v.visit(*this); }
inline void BinLogicOpNode::accept(Visitor& v) { v.visit(*this); }
inline void ValueNode::accept(Visitor& v)      { v.visit(*this); }
inline void UnOpNode::accept(Visitor& v)       { v.visit(*this); }
inline void AssignNode::accept(Visitor& v)     { v.visit(*this); }
inline void VarNode::accept(Visitor& v)        { v.visit(*this); }
inline void IfNode::accept(Visitor& v)         { v.visit(*this); }
inline void WhileNode::accept(Visitor& v)      { v.visit(*this); }
inline void InputNode::accept(Visitor& v)      { v.visit(*this); }
inline void PrintNode::accept(Visitor& v)      { v.visit(*this); }
inline void ExprNode::accept(Visitor& v)       { v.visit(*this); }
inline void ScopeNode::accept(Visitor& v)      { v.visit(*this); }
inline void VarDeclNode::accept(Visitor& v)    { v.visit(*this); }

} // namespace ast
