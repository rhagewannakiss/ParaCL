#include "Visitors/detail/VarTable.hpp"
#include "errors-output/error-formatter.hpp"

#include <stdexcept>

namespace ast {

VarTable::VarTable()
{
    scopes_.emplace_back();
}

void VarTable::enter_scope()
{
    scopes_.emplace_back();
}

void VarTable::leave_scope(const SourceRange& loc)
{
    if (scopes_.size() <= 1) {
        throw std::runtime_error(
            err::format_error(loc, "Trying to leave from global scope"));
    }
    scopes_.pop_back();
}

void VarTable::declare_in_cur_scope(const std::string& name,
                                    int64_t value,
                                    const SourceRange& loc)
{
    auto& cur = scopes_.back();
    const auto iter = cur.find(name);
    if (iter != cur.end()) {
        throw std::runtime_error(
            err::format_error(loc, "Variable " + name + " already declared"));
    }
    cur[name] = value;
}

int64_t VarTable::lookup(const std::string& name, const SourceRange& loc)
{
    for (size_t i = scopes_.size(); i-- > 0;) {
        auto& cur = scopes_[i];
        const auto iter = cur.find(name);
        if (iter != cur.end()) {
            return iter->second;
        }
    }
    throw std::runtime_error(
        err::format_error(loc, "Undefined variable: " + name));
}

void VarTable::assign_or_create(const std::string& name, int64_t value)
{
    for (size_t i = scopes_.size(); i-- > 0;) {
        auto& cur = scopes_[i];
        const auto iter = cur.find(name);
        if (iter != cur.end()) {
            iter->second = value;
            return;
        }
    }
    scopes_.back()[name] = value;
}

} // namespace ast
