%language "c++"
%skeleton "lalr1.cc"
%defines
%define api.value.type variant
%define parse.error verbose
%locations

%param {yy::NumDriver* driver}

%code requires
{
#include <algorithm>
#include <string>
#include <vector>

namespace yy { class NumDriver; }
}

%code
{
#include "driver.hpp"

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,
                         parser::location_type* yylloc,
                         NumDriver* driver);
}
}

%token
    PLUS                 "+"
    MINUS                "-"
    DIV                  "/"
    MUL                  "*"
    MODULUS              "%"
    ASSIGNMENT           "="
    SEMICOLON            ";"
    COMMA                ","
    LESS                 "<"
    GREATER              ">"
    LESS_OR_EQUAL        "<="
    GREATER_OR_EQUAL     ">="
    QUESTION_MARK        "?"
    NOT                  "!"
    EQUAL                "=="
    NOT_EQUAL            "!="
    AND                  "&&"
    OR                   "||"
    XOR                  "^"
    RIGHT_PAREN          ")"
    LEFT_PAREN           "("
    RIGHT_CURLY_BRACKET  "}"
    LEFT_CURLY_BRACKET   "{"
    IF                  "if"
    ELSE                "else"
    WHILE               "while"
    PRINT               "print"
    INPUT               "input"
    NEWLINE
    ERR
;

%token <int> NUMBER
%token <std::string> VAR

%nterm <int> expr
%nterm <int> stmt
%nterm <int> stmts
%nterm <int> program

%right ASSIGNMENT
%left OR
%left XOR
%left AND
%nonassoc EQUAL NOT_EQUAL
%nonassoc LESS GREATER LESS_OR_EQUAL GREATER_OR_EQUAL
%left PLUS MINUS
%left MUL DIV MODULUS
%right UMINUS NOT
%nonassoc XIF
%nonassoc ELSE

%start program

%%

program: stmts
    {
    }
;

stmts: stmt stmts
    {
        $$ = $2;
    }
    | %empty
    {
        $$ = 0;
    }
;

stmt: expr SEMICOLON
    {
        $$ = $1;
    }
    | SEMICOLON
    {
    }
    | IF LEFT_PAREN expr RIGHT_PAREN stmt %prec XIF
    {
        if ($3) {
            = $5;
        } else {
            = 0;
        }
    }
    | IF LEFT_PAREN expr RIGHT_PAREN stmt ELSE stmt
    {
        if ($3) {
            = $5;
        } else {
            = $7;
        }
    }
    | WHILE LEFT_PAREN expr RIGHT_PAREN stmt
    {
        int result = 0;
        while ($3) {
            result = $5;
        }
        $$ = result;
    }
    | LEFT_CURLY_BRACKET stmts RIGHT_CURLY_BRACKET
    {
        $$ = $2;
    }
    | PRINT expr SEMICOLON
    {
        std::cout << $2 << std::endl;
    }
    | NEWLINE
    {
        driver->newline();
    }
;

expr: expr PLUS expr
    {
        $$ = $1 + $3;
    }
    | expr MINUS expr
    {
        $$ = $1 - $3;
    }
    | expr MUL expr
    {
        $$ = $1 * $3;
    }
    | expr DIV expr
    {
        $$ = $1 / $3;
    }
    | expr MODULUS expr
    {
        $$ = $1 % $3;
    }
    | expr EQUAL expr
    {
        $$ = ($1 == $3);
    }
    | expr NOT_EQUAL expr
    {
        $$ = ($1 != $3);
    }
    | expr LESS expr
    {
        $$ = ($1 < $3);
    }
    | expr GREATER expr
    {
        $$ = ($1 > $3);
    }
    | expr LESS_OR_EQUAL expr
    {
        $$ = ($1 <= $3);
    }
    | expr GREATER_OR_EQUAL expr
    {
        $$ = ($1 >= $3);
    }
    | expr AND expr
    {
        $$ = ($1 && $3);
    }
    | expr OR expr
    {
        $$ = ($1 || $3);
    }
    | expr XOR expr
    {
        $$ = ($1 ^ $3);
    }
    | NOT expr
    {
        $$ = !$2;
    }
    | MINUS expr %prec UMINUS
    {
        $$ = -$2;
    }
    | PLUS expr %prec UMINUS
    {
        $$ = $2;
    }
    | LEFT_PAREN expr RIGHT_PAREN
    {
        $$ = $2;
    }
    | NUMBER
    {
        $$ = $1;
    }
    | VAR
    {
        $$ = driver->get_var($1);
    }
    | VAR ASSIGNMENT expr
    {
        driver->set_var($1, $3);
        $$ = $3;
    }
    | INPUT
    {
        int val;
        std::cin >> val;
        $$ = val;
    }
;

%%

namespace yy {

parser::token_type yylex(parser::semantic_type* yylval,
                         parser::location_type* yylloc,
                         NumDriver* driver)
{
    return driver->yylex(yylval, yylloc);
}

void parser::error(const location_type& loc, const std::string& msg) {
    std::cerr << "Error at " << loc.begin.line << ":" << loc.begin.column
              << ": " << msg << std::endl;
}

} // namespace yy