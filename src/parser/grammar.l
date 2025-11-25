%option C++

%{

#include <iostream>

%}

VAR               [a-zA-Z_][a-zA-Z_0-9]*
NLINE             [\n]+
WS                [\t\r\f\v]+
NUMBER            [0-9]+

%%

{NLINE}             {//TODO }
"//".*              {/* skip comments */}
{WHSP}              {/* skip whitespace */}
"+"                 { return yy::parser::token_type::PLUS; }
"-"                 { return yy::parser::token_type::MINUS; }
"/"                 { return yy::parser::token_type::DIV; }
"*"                 { return yy::parser::token_type::MUL; }
"%"                 { return yy::parser::token_type::MODULUS; }
"="                 { return yy::parser::token_type::ASSIGNMENT; }
";"                 { return yy::parser::token_type::SEMICOLON; }
","                 { return yy::parser::token_type::COLON; }
"<"                 { return yy::parser::token_type::LESS; }
">"                 { return yy::parser::token_type::GREATER; }
"<="                { return yy::parser::token_type::LESS_OR_EQUAL; }
">="                { return yy::parser::token_type::GREATER_OR_EQUAL; }
"?"                 { return yy::parser::token_type::QUESTION_MARK; }
"!"                 { return yy::parser::token_type::NOT; }
"=="                { return yy::parser::token_type::EQUAL; }
"!="                { return yy::parser::token_type::NOT_EQUAL; }
"&&"                { return yy::parser::token_type::AND; }
"||"                { return yy::parser::token_type::OR; }
"^"                 { return yy::parser::token_type::XOR; }
")"                 { return yy::parser::token_type::RIGHT_ROUND_BRACKER; }
"("                 { return yy::parser::token_type::LEFT_ROUND_BRACKER; }
"}"                 { return yy::parser::token_type::RIGHT_CURLY_BRACKER; }
"{"                 { return yy::parser::token_type::LEFT_CURLY_BRACKER; }
"if"                { return yy::parser::token_type::IF; }
"else"              { return yy::parser::token_type::ELSE; }
"while"             { return yy::parser::token_type::WHILE; }
{NUMBER}            { return yy::parser::token_type::NUMBER; }
{VAR}               { return yy::parser::token_type::VAR; }
.			        { return yy::parser::token_type::ERR; }

%%