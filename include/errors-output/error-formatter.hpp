#pragma once

#include <AST/AST.hpp>

namespace err {
inline std::string format_error(const ast::SourceRange& loc,
                                const std::string& message)
{
    const std::string location = loc.make_string();
    if (location.empty()) {
        return "error: " + message;
    }
    return location + ": error: " + message;
}
} // namespace err
