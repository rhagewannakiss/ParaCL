#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "grammar.tab.hh"
#include <FlexLexer.h>

#include <iostream>
#include <string>
#include <utility>

#include "AST/AST.hpp"
#include "driver/location_utils.hpp"
#include "errors-output/error-formatter.hpp"

namespace yy {

class NumDriver
{
private:
    FlexLexer* plex_;
    std::string filename_;
    location loc_;
    ast::AST ast_;
    int error_cnt_ = 0;

public:
    explicit NumDriver(FlexLexer* plex, std::string fn = "")
      : plex_(plex)
      , filename_(std::move(fn))
    {
        loc_.initialize(&filename_);
    }

    parser::token_type yylex(parser::semantic_type* yylval,
                             parser::location_type* yylloc)
    {
        loc_.step();

        parser::token_type tt = static_cast<parser::token_type>(plex_->yylex());

        loc_.columns(plex_->YYLeng());
        *yylloc = loc_;

        if (tt == parser::token_type::NUMBER)
            yylval->emplace<int64_t>(std::stoll(plex_->YYText()));
        if (tt == parser::token_type::VAR)
            yylval->emplace<std::string>(plex_->YYText());

        return tt;
    }

    void add_error(const location& loc, const std::string& msg)
    {
        auto range = to_source_range(loc);
        std::cerr << err::format_error(range, msg) << std::endl;
        error_cnt_++;
    }

    bool has_errors() const
    {
        return error_cnt_ > 0;
    }

    bool parse()
    {
        parser parser(this);
        bool res = parser.parse();
        return !res;
    }

    void newline()
    {
        loc_.lines(1);
        loc_.step();
    }

    const location& get_location() const
    {
        return loc_;
    }

    void set_ast_root(std::unique_ptr<ast::BaseNode> root)
    {
        ast_.set_root(std::move(root));
    }

    const ast::AST& get_ast() const
    {
        return ast_;
    }
};

} // namespace yy

#endif // DRIVER_HPP
