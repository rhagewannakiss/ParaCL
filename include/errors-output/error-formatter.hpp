#pragma once

#include <AST/AST.hpp>
#include <string>
#include <string_view>

namespace err {
inline std::string format_error(const ast::SourceRange& loc,
                                std::string_view message)
{
    const std::string location = loc.make_string();
    std::string prefix;
    if (location.empty()) {
        prefix = "error: ";
    } else {
        prefix = location + ": error: ";
    }
    prefix += message;
    return prefix;
}
} // namespace err
