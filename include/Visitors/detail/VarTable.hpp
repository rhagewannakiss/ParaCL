#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "AST/SourceRange.hpp"

namespace ast {

class VarTable
{
    using scope = std::unordered_map<std::string, int64_t>;
    std::vector<scope> scopes_;

public:
    VarTable();

    void enter_scope();
    void leave_scope(const SourceRange& loc = {});

    void declare_in_cur_scope(std::string_view name,
                              int64_t value = 0,
                              const SourceRange& loc = {});

    int64_t lookup(std::string_view name, const SourceRange& loc = {});
    void assign_or_create(std::string_view name, int64_t value);
};

} // namespace ast
