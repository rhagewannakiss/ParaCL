%option C++
%option noyywrap yylineno

%{

#include "grammar.tab.hh"

#include <iostream>

%}

VAR               [a-zA-Z_][a-zA-Z_0-9]*
NLINE             [\n]+
WS                [\t\r\f\v]+
NUMBER            [0-9]+

%%

{NLINE}             { return yy::parser::token_type::NEWLINE;}
"//".*              {/* skip comments */}
{WS}                {/* skip whitespace */}
"+"                 { return yy::parser::token_type::PLUS; }
"-"                 { return yy::parser::token_type::MINUS; }
"/"                 { return yy::parser::token_type::DIV; }
"*"                 { return yy::parser::token_type::MUL; }
"%"                 { return yy::parser::token_type::MODULUS; }
"="                 { return yy::parser::token_type::ASSIGNMENT; }
";"                 { return yy::parser::token_type::SEMICOLON; }
","                 { return yy::parser::token_type::COMMA; }
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
")"                 { return yy::parser::token_type::RIGHT_PAREN; }
"("                 { return yy::parser::token_type::LEFT_PAREN; }
"}"                 { return yy::parser::token_type::RIGHT_CURLY_BRACKET; }
"{"                 { return yy::parser::token_type::LEFT_CURLY_BRACKET; }
"if"                { return yy::parser::token_type::IF; }
"for"               { return yy::parser::token_type::FOR; }
"else"              { return yy::parser::token_type::ELSE; }
"while"             { return yy::parser::token_type::WHILE; }
"print"             { return yy::parser::token_type::PRINT; }
{NUMBER}            { return yy::parser::token_type::NUMBER; }
{VAR}               { return yy::parser::token_type::VAR; }
.			        { return yy::parser::token_type::ERR; }

%%