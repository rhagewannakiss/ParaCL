#pragma once

#include <cstdint>
#include <string>

namespace ast {

struct SourceRange {
    std::string file;
    std::uint32_t begin_line     = 0;
    std::uint32_t begin_column   = 0;
    std::uint32_t end_line       = 0;
    std::uint32_t end_column     = 0;
};

} // namespace ast
