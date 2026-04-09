#include "Visitors/detail/VarTable.hpp"
#include "errors-output/error-formatter.hpp"

#include <cassert>
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
    (void)loc;
    assert(scopes_.size() > 1 &&
           "Trying to leave from global scope in VarTable::leave_scope");
    if (scopes_.size() <= 1) {
        return;
    }
    scopes_.pop_back();
}

void VarTable::declare_in_cur_scope(std::string_view name,
                                    int64_t value,
                                    const SourceRange& loc)
{
    const std::string key(name);
    auto& cur = scopes_.back();
    const auto iter = cur.find(key);
    if (iter != cur.end()) {
        throw std::runtime_error(
            err::format_error(loc, "Variable " + key + " already declared"));
    }
    cur[key] = value;
}

int64_t VarTable::lookup(std::string_view name, const SourceRange& loc)
{
    const std::string key(name);
    for (size_t i = scopes_.size(); i-- > 0;) {
        auto& cur = scopes_[i];
        const auto iter = cur.find(key);
        if (iter != cur.end()) {
            return iter->second;
        }
    }

    throw std::runtime_error(
        err::format_error(loc, "Undefined variable: " + key));
}

void VarTable::assign_or_create(std::string_view name, int64_t value)
{
    const std::string key(name);
    for (size_t i = scopes_.size(); i-- > 0;) {
        auto& cur = scopes_[i];
        const auto iter = cur.find(key);
        if (iter != cur.end()) {
            iter->second = value;
            return;
        }
    }
    scopes_.back()[key] = value;
}

} // namespace ast
