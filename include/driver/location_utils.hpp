#pragma once

#include "AST/AST.hpp"
#include "AST/SourceRange.hpp"
#include "grammar.tab.hh"

inline ast::SourceRange to_source_range(const yy::parser::location_type& loc) {
    ast::SourceRange range;
    range.begin_line = static_cast<std::uint32_t>(loc.begin.line);
    range.begin_column = static_cast<std::uint32_t>(loc.begin.column);
    range.end_line = static_cast<std::uint32_t>(loc.end.line);
    range.end_column = static_cast<std::uint32_t>(loc.end.column);
    if (loc.begin.filename) {
        range.file = *loc.begin.filename;
    }
    return range;
}

inline ast::BaseNode::NodePtr with_loc(
        ast::BaseNode::NodePtr node, 
        const yy::parser::location_type& loc,
        yy::NumDriver* driver) 
{
    if (node) {
        node->set_location(to_source_range(loc));
    }
    return node;
}
