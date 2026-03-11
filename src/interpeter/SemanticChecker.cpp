#include "Visitors/SemanticChecker.hpp"
#include "errors-output/error-formatter.hpp"

#include <iostream>

namespace ast {

SemanticChecker::SemanticChecker()
{
    scopes_.emplace_back();
}

void SemanticChecker::check(BaseNode* root)
{
    if (root)
        root->accept(*this);
}

void SemanticChecker::printErrors(std::ostream& out) const
{
    for (const auto& e : errors_)
        out << e << std::endl;
}

void SemanticChecker::enterScope()
{
    scopes_.emplace_back();
}

void SemanticChecker::leaveScope(const SourceRange& loc)
{
    if (scopes_.size() <= 1) {
        addError(loc, "Internal error: trying to leave global scope");
        return;
    }
    scopes_.pop_back();
}

void SemanticChecker::declareVariable(const std::string& name,
                                      const SourceRange& loc)
{
    auto& cur = scopes_.back();
    if (cur.find(name) != cur.end()) {
        addError(loc, "Variable '" + name + "' already declared in this scope");
        return;
    }
    cur.insert(name);
}

bool SemanticChecker::isDeclared(const std::string& name) const
{
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->find(name) != it->end())
            return true;
    }
    return false;
}

void SemanticChecker::addError(const SourceRange& loc, const std::string& msg)
{
    errors_.push_back(err::format_error(loc, msg));
}

void SemanticChecker::visit(VarNode& node)
{
    if (!isDeclared(node.name())) {
        addError(node.location(), "Undefined variable: " + node.name());
    }
}

void SemanticChecker::visit(AssignNode& node)
{
    auto* lhs = node.lhs();
    if (!lhs || lhs->node_type() != base_node_type::var) {
        addError(node.location(),
                 "Left-hand side of assignment must be a variable");
    } else {
        auto* var = static_cast<VarNode*>(lhs);
        if (!isDeclared(var->name())) {
            declareVariable(var->name(), var->location());
        }
    }
    if (auto* rhs = node.rhs())
        rhs->accept(*this);
}

void SemanticChecker::visit(VarDeclNode& node)
{
    declareVariable(node.name(), node.location());
    if (auto* init = node.init_expr())
        init->accept(*this);
}

void SemanticChecker::visit(ScopeNode& node)
{
    enterScope();
    for (const auto& stmt : node.statements()) {
        stmt->accept(*this);
    }
    leaveScope(node.location());
}

void SemanticChecker::visit(IfNode& node)
{
    enterScope();
    if (auto* cond = node.condition())
        cond->accept(*this);
    if (auto* thenBr = node.then_branch())
        thenBr->accept(*this);
    if (auto* elseBr = node.else_branch())
        elseBr->accept(*this);
    leaveScope(node.location());
}

void SemanticChecker::visit(WhileNode& node)
{
    enterScope();
    if (auto* cond = node.condition())
        cond->accept(*this);
    if (auto* body = node.body())
        body->accept(*this);
    leaveScope(node.location());
}

void SemanticChecker::visit(ForNode& node)
{
    enterScope();
    if (auto* init = node.get_init())
        init->accept(*this);
    if (auto* cond = node.get_cond())
        cond->accept(*this);
    if (auto* body = node.get_body())
        body->accept(*this);
    if (auto* step = node.get_step())
        step->accept(*this);
    leaveScope(node.location());
}

void SemanticChecker::visit(BinArithOpNode& node)
{
    if (auto* left = node.left())
        left->accept(*this);
    if (auto* right = node.right())
        right->accept(*this);
}

void SemanticChecker::visit(BinLogicOpNode& node)
{
    if (auto* left = node.left())
        left->accept(*this);
    if (auto* right = node.right())
        right->accept(*this);
}

void SemanticChecker::visit(UnOpNode& node)
{
    if (auto* operand = node.operand())
        operand->accept(*this);
}

void SemanticChecker::visit(ExprNode& node)
{
    if (auto* expr = node.expr())
        expr->accept(*this);
}

void SemanticChecker::visit(PrintNode& node)
{
    if (auto* expr = node.expr())
        expr->accept(*this);
}

void SemanticChecker::visit(InputNode&)
{
}
void SemanticChecker::visit(ValueNode&)
{
}
void SemanticChecker::visit(ErrorNode&)
{
}
void SemanticChecker::visit(EmptyNode&)
{
}

} // namespace ast
