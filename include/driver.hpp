#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "grammar.tab.hh"
#include <FlexLexer.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace yy {

class NumDriver {
private:
    FlexLexer* plex_;
    location loc_;
    std::map<std::string, int> variables_;
    std::vector<std::pair<std::vector<int>, std::vector<int>>> equations_;

public:
    explicit NumDriver(FlexLexer* plex) : plex_(plex) {}

    parser::token_type yylex(parser::semantic_type* yylval,
                             parser::location_type* yylloc) {
        loc_.step();

        parser::token_type tt = static_cast<parser::token_type>(plex_->yylex());

        loc_.columns(plex_->YYLeng());
        *yylloc = loc_;

        switch (tt) {
            case parser::token_type::NUMBER:
                yylval->emplace<int>(std::stoi(plex_->YYText()));
                break;
            case parser::token_type::VAR:
                yylval->emplace<std::string>(plex_->YYText());
                break;
            default:
                break;
        }

        return tt;
    }

    bool parse() {
        parser parser(this);
        bool res = parser.parse();
        return !res;
    }

    void newline() {
        loc_.lines(1);
        loc_.step();
    }

    const location& get_location() const {
        return loc_;
    }

    void insert(std::vector<std::pair<std::vector<int>, std::vector<int>>> v) {
        equations_.assign(v.rbegin(), v.rend());
    }

    void printout() const {
        for (const auto& eq : equations_) {
            int sum_left = 0;
            for (int val : eq.first) {
                sum_left += val;
            }
            int sum_right = 0;
            for (int val : eq.second) {
                sum_right += val;
            }
            std::cout << "Checking: " << sum_left << " vs " << sum_right
                      << "; Result: " << (sum_left == sum_right) << std::endl;
        }
    }
};

} // namespace yy

#endif // DRIVER_HPP