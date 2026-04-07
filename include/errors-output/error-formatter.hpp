#pragma once

#include <AST/AST.hpp>
#include <format>
#include <string>
#include <string_view>

namespace err {
inline std::string format_error(const ast::SourceRange& loc,
                                std::string_view message)
{
    const std::string location = loc.make_string();
    if (location.empty()) {
        return std::format("error: {}", message);
    }
    return std::format("{}: error: {}", location, message);
}
} // namespace err
