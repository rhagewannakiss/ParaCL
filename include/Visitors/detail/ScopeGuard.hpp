#pragma once

#include "AST/SourceRange.hpp"
#include "Visitors/detail/VarTable.hpp"

namespace ast::detail {

class ScopeGuard
{
    VarTable& table_;
    SourceRange location_;

public:
    explicit ScopeGuard(VarTable& table, SourceRange location = {});
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ~ScopeGuard() noexcept;
};

} // namespace ast::detail
