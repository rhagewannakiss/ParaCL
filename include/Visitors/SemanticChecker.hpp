#pragma once

#include "AST/SourceRange.hpp"
#include "Visitors/Visitor.hpp"
#include <set>
#include <string>
#include <vector>

namespace ast {

class SemanticChecker : public Visitor
{
public:
    SemanticChecker();

    void check(BaseNode* root);

    bool hasErrors() const
    {
        return !errors_.empty();
    }

    void printErrors(std::ostream& out) const;

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
    std::vector<std::string> errors_;
    std::vector<std::set<std::string>> scopes_;

    void enterScope();
    void leaveScope(const SourceRange& loc);
    void declareVariable(const std::string& name, const SourceRange& loc);
    bool isDeclared(const std::string& name) const;
    void addError(const SourceRange& loc, const std::string& msg);
};

} // namespace ast