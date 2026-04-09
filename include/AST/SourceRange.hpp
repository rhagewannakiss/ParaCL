#pragma once

#include <cstdint>
#include <format>
#include <limits>
#include <string>

namespace ast {

struct SourceRange
{
    std::string file;

    static constexpr std::uint64_t kInvalidPos =
        std::numeric_limits<std::uint64_t>::max();
    std::uint64_t begin_line = kInvalidPos;
    std::uint64_t begin_column = kInvalidPos;
    std::uint64_t end_line = kInvalidPos;
    std::uint64_t end_column = kInvalidPos;

    bool has_valid_point() const
    {
        return begin_line != kInvalidPos && begin_column != kInvalidPos;
    }

    bool has_gcc_location() const
    {
        return !file.empty() && has_valid_point();
    }

    std::string make_string() const
    {
        if (!has_gcc_location()) {
            return "";
        }
        return std::format("{}:{}:{}", file, begin_line, begin_column);
    }
};

} // namespace ast
