#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "AST/SourceRange.hpp"
#include "errors-output/error-formatter.hpp"

namespace ast::parser::detail {

class SymbolTable
{
    using scope = std::unordered_set<std::string>;
    std::vector<scope> scopes_;

public:
    SymbolTable()
    {
        scopes_.emplace_back();
    }

    void enter_scope()
    {
        scopes_.emplace_back();
    }
    void leave_scope(const SourceRange& loc = {})
    {
        if (scopes_.size() <= 1) {
            throw std::runtime_error(
                err::format_error(loc, "Trying to leave from global scope"));
        }
        scopes_.pop_back();
    }

    void declare_in_cur_scope(std::string_view name,
                              const SourceRange& loc = {})
    {
        const std::string key(name);
        auto& cur = scopes_.back();
        const auto iter = cur.find(key);
        if (iter != cur.end()) {
            throw std::runtime_error(err::format_error(
                loc, "Variable " + key + " already declared"));
        }
        cur.insert(key);
    }

    bool lookup(std::string_view name, const SourceRange& loc = {})
    {
        const std::string key(name);
        for (size_t i = scopes_.size(); i-- > 0;) {
            auto& cur = scopes_[i];
            const auto iter = cur.find(key);
            if (iter != cur.end()) {
                return true;
            }
        }
        throw std::runtime_error(
            err::format_error(loc, "Undefined variable: " + key));
    }
};

} // namespace ast::detail
