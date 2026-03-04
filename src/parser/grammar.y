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
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "AST/AST.hpp"
#include "Visitors/Visitor.hpp"

namespace yy { class NumDriver; }
using NodePtr = std::unique_ptr<ast::BaseNode>;
}

%code
{
#include "driver/driver.hpp"
#include "driver/location_utils.hpp"

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
    IF                   "if"
    FOR                  "for"
    ELSE                 "else"
    WHILE                "while"
    PRINT                "print"
    NEWLINE
    ERR
;

%token <int64_t> NUMBER
%token <std::string> VAR

%nterm <std::unique_ptr<ast::BaseNode>> expr
%nterm <std::unique_ptr<ast::BaseNode>> stmt
%nterm <std::unique_ptr<ast::BaseNode>> lvalue
%nterm <std::unique_ptr<ast::BaseNode>> for_init
%nterm <std::unique_ptr<ast::BaseNode>> for_cond
%nterm <std::unique_ptr<ast::BaseNode>> for_step
%nterm <std::unique_ptr<ast::BaseNode>> for_step_expr
%nterm <std::unique_ptr<ast::BaseNode>> program
%nterm <std::vector<std::unique_ptr<ast::BaseNode>>> stmts

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
        auto scope = std::make_unique<ast::ScopeNode>();
        for (size_t i = $1.size(); i-- > 0;) {
            if ($1[i]) {
                scope->add_statement(std::move($1[i]));
            }
        }
        driver->set_ast_root(with_loc(std::move(scope), @$));
    }
;

stmts: stmt stmts
    {
        $$ = std::move($2);
        if ($1) {
            $$.push_back(std::move($1));
        }
    }
    | %empty
    {
        $$ = std::vector<std::unique_ptr<ast::BaseNode>>();
    }
;

stmt: expr SEMICOLON
    {
        $$ = with_loc(std::make_unique<ast::ExprNode>(std::move($1)), @$);
    }
    | expr error
    {
        error(@2, "No semicolon");
        $$ = with_loc(std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(0)), @$);
    }
    | SEMICOLON
    {
        $$ = nullptr;
    }
    | VAR SEMICOLON
    {
        $$ = with_loc(std::make_unique<ast::VarDeclNode>($1), @$);
    }
    | VAR error
    {
        error(@2, "No semicolon");
        $$ = with_loc(std::make_unique<ast::VarDeclNode>($1), @$);
    }
    | IF LEFT_PAREN expr RIGHT_PAREN stmt %prec XIF
    {
        $$ = with_loc(std::make_unique<ast::IfNode>(std::move($3), std::move($5)), @$);
    }
    | IF LEFT_PAREN error RIGHT_PAREN stmt %prec XIF
    {
        error(@3, "Missing condition in if");
        $$ = with_loc(std::make_unique<ast::IfNode>(std::make_unique<ast::ValueNode>(0), std::move($5)), @$);
    }
    | IF LEFT_PAREN expr RIGHT_PAREN stmt ELSE stmt
    {
        $$ = with_loc(std::make_unique<ast::IfNode>(std::move($3), std::move($5), std::move($7)), @$);
    }
    | IF LEFT_PAREN error RIGHT_PAREN stmt ELSE stmt
    {
        error(@3, "Missing condition in if-else");
        $$ = with_loc(std::make_unique<ast::IfNode>(std::make_unique<ast::ValueNode>(0), std::move($5), std::move($7)), @$);
    }
    | WHILE LEFT_PAREN expr RIGHT_PAREN stmt
    {
        $$ = with_loc(std::make_unique<ast::WhileNode>(std::move($3), std::move($5)), @$);
    }
    | WHILE LEFT_PAREN error RIGHT_PAREN stmt
    {
        error(@3, "Missing condition in while");
        $$ = with_loc(std::make_unique<ast::WhileNode>(std::make_unique<ast::ValueNode>(0), std::move($5)), @$);
    }
    | FOR LEFT_PAREN for_init for_cond for_step RIGHT_PAREN stmt
    {
        std::unique_ptr<ast::BaseNode> body;

        if ($7 && $7->node_type() == ast::base_node_type::scope) {
            body = std::move($7);
        } else {
            auto scope = std::make_unique<ast::ScopeNode>();
            if ($7) {
                scope->add_statement(std::move($7));
            }
            body = std::move(scope);
        }

        $$ = with_loc(std::make_unique<ast::ForNode>(std::move($3), std::move($4), std::move($5), std::move(body)), @$);
    }
    | FOR LEFT_PAREN error RIGHT_PAREN stmt
    {
        error(@3, "Invalid for loop header");
        std::unique_ptr<ast::BaseNode> body;

        if ($5 && $5->node_type() == ast::base_node_type::scope) {
            body = std::move($5);
        } else {
            auto scope = std::make_unique<ast::ScopeNode>();
            if ($5) {
                scope->add_statement(std::move($5));
            }
            body = std::move(scope);
        }

        $$ = with_loc(std::make_unique<ast::ForNode>(nullptr, nullptr, nullptr, std::move(body)), @$);
    }
    | LEFT_CURLY_BRACKET stmts RIGHT_CURLY_BRACKET
    {
        auto scope = std::make_unique<ast::ScopeNode>();
        for (size_t i = $2.size(); i-- > 0;) {
            if ($2[i]) {
                static_cast<ast::ScopeNode*>(scope.get())->add_statement(std::move($2[i]));
            }
        }
        $$ = with_loc(std::move(scope), @$);
    }
    | PRINT expr SEMICOLON
    {
        $$ = with_loc(std::make_unique<ast::PrintNode>(std::move($2)), @$);
    }
    | PRINT expr error
    {
        error(@3, "Missing semicolon in print");
        $$ = with_loc(std::make_unique<ast::PrintNode>(std::make_unique<ast::ValueNode>(0)), @$);
    }
;

lvalue: VAR
    {
        $$ = with_loc(std::make_unique<ast::VarNode>($1), @$);
    }
;

for_init: expr SEMICOLON
    {
        $$ = std::move($1);
    }
    | SEMICOLON
    {
        $$ = nullptr;
    }
;

for_cond: expr SEMICOLON
    {
        $$ = std::move($1);
    }
    | error SEMICOLON
    {
        error(@1, "Missing condition in for");
        $$ = with_loc(std::make_unique<ast::ValueNode>(0), @$);
    }
;

for_step: for_step_expr
    {
        $$ = std::move($1);
    }
    | %empty
    {
        $$ = nullptr;
    }
;

for_step_expr: lvalue ASSIGNMENT expr
    {
        $$ = with_loc(std::make_unique<ast::AssignNode>(std::move($1), std::move($3)), @$);
    }
;

expr: expr PLUS expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::add, std::move($1), std::move($3)), @$);
    }
    | error PLUS expr
    {
        error(@1, "Missing left operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::add, std::make_unique<ast::ValueNode>(0), std::move($3)), @$);
    }
    | expr PLUS error
    {
        error(@3, "Missing right operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::add, std::move($1), std::make_unique<ast::ValueNode>(0)), @$);
    }
    | expr MINUS expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::sub, std::move($1), std::move($3)), @$);
    }
    | error MINUS expr
    {
        error(@1, "Missing left operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::sub, std::make_unique<ast::ValueNode>(0), std::move($3)), @$);
    }
    | expr MINUS error
    {
        error(@3, "Missing right operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::sub, std::move($1), std::make_unique<ast::ValueNode>(0)), @$);
    }
    | expr MUL expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::mul, std::move($1), std::move($3)), @$);
    }
    | expr DIV expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::div,
                std::move($1),
                std::move($3)),
            @$);
    }
    | expr MODULUS expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(
                ast::bin_arith_op_type::mod,
                std::move($1),
                std::move($3)),
            @$);
    }
    | expr EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::equal, std::move($1), std::move($3)), @$);
    }
    | expr NOT_EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::not_equal, std::move($1), std::move($3)), @$);
    }
    | expr LESS expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::less, std::move($1), std::move($3)), @$);
    }
    | expr GREATER expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::greater, std::move($1), std::move($3)), @$);
    }
    | expr LESS_OR_EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::less_equal, std::move($1), std::move($3)), @$);
    }
    | expr GREATER_OR_EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::greater_equal, std::move($1), std::move($3)), @$);
    }
    | expr AND expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::logical_and, std::move($1), std::move($3)), @$);
    }
    | expr OR expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::logical_or, std::move($1), std::move($3)), @$);
    }
    | expr XOR expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::bitwise_xor, std::move($1), std::move($3)), @$);
    }
    | NOT expr
    {
        $$ = with_loc(std::make_unique<ast::UnOpNode>(ast::unop_node_type::logical_not, std::move($2)), @$);
    }
    | MINUS expr %prec UMINUS
    {
        $$ = with_loc(std::make_unique<ast::UnOpNode>(ast::unop_node_type::neg, std::move($2)), @$);
    }
    | PLUS expr %prec UMINUS
    {
        $$ = with_loc(std::make_unique<ast::UnOpNode>(ast::unop_node_type::pos, std::move($2)), @$);
    }
    | LEFT_PAREN expr RIGHT_PAREN
    {
        $$ = std::move($2);
    }
    | NUMBER
    {
        $$ = with_loc(std::make_unique<ast::ValueNode>($1), @$);
    }
    | VAR
    {
        $$ = with_loc(std::make_unique<ast::VarNode>(std::move($1)), @$);
    }
    | lvalue ASSIGNMENT expr
    {
        $$ = with_loc(std::make_unique<ast::AssignNode>(std::move($1), std::move($3)), @$);
    }
    | QUESTION_MARK
    {
        $$ = with_loc(std::make_unique<ast::InputNode>(), @$);
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
     driver->add_error(loc, msg);
}

} // namespace yy