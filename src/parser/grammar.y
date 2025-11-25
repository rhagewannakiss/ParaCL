%language "c++"

%skeleton "lalr1.cc"
%defines
%define api.value.type variant
%locations
%define api.location.file "location.hh"
%param {yy::Driver* driver}

%code requires
{
#include <algorithm>
#include <string>
#include <vector>

namespace yy { class Driver; }
}

%code
{
#include "driver.hpp"

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,
                         Driver* driver);
}
}

%token
  PLUS                "+"
  MINUS               "-"
  DIV                 "/"
  MUL                 "*"
  MODULUS             "%"
  ASSIGNMENT          "="
  SEMICOLON           ";"
  COLON               ","
  LESS                "<"
  GREATER             ">"
  LESS_OR_EQUAL       "<="
  GREATER_OR_EQUAL    ">="
  QUESTION_MARK       "?"
  NOT                 "!"
  EQUAL               "=="
  NOT_EQUAL           "!="
  AND                 "&&"
  OR                  "||"
  XOR                 "^"
  RIGHT_ROUND_BRACKER ")"
  LEFT_ROUND_BRACKER  "("
  RIGHT_CURLY_BRACKER "}"
  LEFT_CURLY_BRACKER  "{"
  IF                  "if"
  ELSE                "else"
  WHILE               "while"
  ERR
;

%token <int> NUMBER
%token <std::string> VAR

%nterm <std::vector<int>> expr
%nterm <std::pair<std::vector<int>, std::vector<int>>> equals
%nterm <std::vector<std::pair<std::vector<int>, std::vector<int>>>> eqlist

%left XOR
%left OR
%left AND
%left PLUS MINUS
%left MUL DIV MODULUS
%right ASSIGNMENT
%nonassoc EQUAL NOT_EQUAL BELOW GREATER EQUAL_OR_BELOW EQUAL_OR_GREATER
%nonassoc UMINUS NOT
%nonassoc XIF
%nonassoc ELSE

%start program

%%

program: eqlist                   { driver->insert($1); }
;

eqlist: equals SEMICOLON eqlist   { $$ = $3; $$.push_back($1); }
      | equals SEMICOLON          { $$.push_back($1);          }
;

equals: expr EQUAL expr           { $$ = std::make_pair($1, $3); }
;

expr: NUMBER                      { $$.push_back($1); }
    | expr PLUS NUMBER            { $$ = $1; $$.push_back($3); }
    | expr MINUS NUMBER           { $$ = $1; $$.push_back(-$3); }
;

%%

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,
                         NumDriver* driver)
{
  return driver->yylex(yylval);
}

void parser::error(const std::string&){}
}