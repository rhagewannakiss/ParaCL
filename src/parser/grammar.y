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

%token <int> NUMBER
%token <std::string> VAR

%nterm <std::unique_ptr<ast::BaseNode>> expr
%nterm <std::unique_ptr<ast::BaseNode>> stmt
%nterm <std::unique_ptr<ast::BaseNode>> lvalue
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
        for (size_t i = 0; i < $1.size(); ++i) {
            scope->add_statement(std::move($1[i]));
        }
        driver->set_ast_root(with_loc(std::move(scope), @$, driver));
    }
;

stmts: stmt stmts
    {
        $$ = std::move($2);
        $$.push_back(std::move($1));
    }
    | %empty
    {
        $$ = std::vector<std::unique_ptr<ast::BaseNode>>();
    }
;

stmt: expr SEMICOLON
    {
        $$ = with_loc(std::make_unique<ast::ExprNode>(std::move($1)), @$, driver);
    }
    | expr error
    {
        error(@2, "No semicolon");
        $$ = with_loc(std::make_unique<ast::ExprNode>(std::make_unique<ast::ValueNode>(0)), @$, driver);
    }
    | SEMICOLON
    {
        $$ = nullptr;
    }
    | VAR SEMICOLON
    {
        $$ = with_loc(std::make_unique<ast::VarDeclNode>($1, std::make_unique<ast::ValueNode>(0)), @$, driver);
    }
    | VAR error
    {
        error(@2, "No semicolon");
        $$ = with_loc(std::make_unique<ast::VarDeclNode>($1, std::make_unique<ast::ValueNode>(0)), @$, driver);
    }
    | IF LEFT_PAREN expr RIGHT_PAREN stmt %prec XIF
    {
        $$ = with_loc(std::make_unique<ast::IfNode>(std::move($3), std::move($5)), @$, driver);
    }
    | IF LEFT_PAREN error RIGHT_PAREN stmt %prec XIF
    {
        error(@3, "Missing condition in if");
        $$ = with_loc(std::make_unique<ast::IfNode>(std::make_unique<ast::ValueNode>(0), std::move($5)), @$, driver);
    }
    | IF LEFT_PAREN expr RIGHT_PAREN stmt ELSE stmt
    {
        $$ = with_loc(std::make_unique<ast::IfNode>(std::move($3), std::move($5), std::move($7)), @$, driver);
    }
    | IF LEFT_PAREN error RIGHT_PAREN stmt ELSE stmt
    {
        error(@3, "Missing condition in if-else");
        $$ = with_loc(std::make_unique<ast::IfNode>(std::make_unique<ast::ValueNode>(0), std::move($5), std::move($7)), @$, driver);
    }
    | WHILE LEFT_PAREN expr RIGHT_PAREN stmt
    {
        $$ = with_loc(std::make_unique<ast::WhileNode>(std::move($3), std::move($5)), @$, driver);
    }
    | WHILE LEFT_PAREN error RIGHT_PAREN stmt
    {
        error(@3, "Missing condition in while");
        $$ = with_loc(std::make_unique<ast::WhileNode>(std::make_unique<ast::ValueNode>(0), std::move($5)), @$, driver);
    }
    | LEFT_CURLY_BRACKET stmts RIGHT_CURLY_BRACKET
    {
        auto scope = std::make_unique<ast::ScopeNode>();
        for (size_t i = 0; i < $2.size(); ++i) {
            static_cast<ast::ScopeNode*>(scope.get())->add_statement(std::move($2[i]));
        }
        $$ = with_loc(std::move(scope), @$, driver);
    }
    | PRINT expr SEMICOLON
    {
        $$ = with_loc(std::make_unique<ast::PrintNode>(std::move($2)), @$, driver);
    }
    | PRINT expr error
    {
        error(@3, "Missing semicolon in print");
        $$ = with_loc(std::make_unique<ast::PrintNode>(std::make_unique<ast::ValueNode>(0)), @$, driver);
    }
    | NEWLINE
    {
        driver->newline();
        $$ = nullptr;
    }
;

lvalue: VAR
    {
        $$ = with_loc(std::make_unique<ast::VarNode>($1), @$, driver);
    }
;

expr: expr PLUS expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::add, std::move($1), std::move($3)), @$, driver);
    }
    | error PLUS expr
    {
        error(@1, "Missing left operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::add, std::make_unique<ast::ValueNode>(0), std::move($3)), @$, driver);
    }
    | expr PLUS error
    {
        error(@3, "Missing right operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::add, std::move($1), std::make_unique<ast::ValueNode>(0)), @$, driver);
    }
    | expr MINUS expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::sub, std::move($1), std::move($3)), @$, driver);
    }
    | error MINUS expr
    {
        error(@1, "Missing left operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::sub, std::make_unique<ast::ValueNode>(0), std::move($3)), @$, driver);
    }
    | expr MINUS error
    {
        error(@3, "Missing right operand");
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::sub, std::move($1), std::make_unique<ast::ValueNode>(0)), @$, driver);
    }
    | expr MUL expr
    {
        $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::mul, std::move($1), std::move($3)), @$, driver);
    }
    | expr DIV expr
    {
        if ($3 == 0) {
            error(@3, "Division by zero");
            $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::div, std::move($1), std::make_unique<ast::ValueNode>(0)), @$, driver);
        } else {
            $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::div, std::move($1), std::move($3)), @$, driver);
        }
    }
    | expr MODULUS expr
    {
        if ($3 == 0) {
            error(@3, "Division by zero");
            $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::div, std::move($1), std::make_unique<ast::ValueNode>(0)), @$, driver);
        } else {
            $$ = with_loc(std::make_unique<ast::BinArithOpNode>(ast::bin_arith_op_type::mod, std::move($1), std::move($3)), @$, driver);
        }
    }
    | expr EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::equal, std::move($1), std::move($3)), @$, driver);
    }
    | expr NOT_EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::not_equal, std::move($1), std::move($3)), @$, driver);
    }
    | expr LESS expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::less, std::move($1), std::move($3)), @$, driver);
    }
    | expr GREATER expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::greater, std::move($1), std::move($3)), @$, driver);
    }
    | expr LESS_OR_EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::less_equal, std::move($1), std::move($3)), @$, driver);
    }
    | expr GREATER_OR_EQUAL expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::greater_equal, std::move($1), std::move($3)), @$, driver);
    }
    | expr AND expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::logical_and, std::move($1), std::move($3)), @$, driver);
    }
    | expr OR expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::logical_or, std::move($1), std::move($3)), @$, driver);
    }
    | expr XOR expr
    {
        $$ = with_loc(std::make_unique<ast::BinLogicOpNode>(ast::bin_logic_op_type::bitwise_xor, std::move($1), std::move($3)), @$, driver);
    }
    | NOT expr
    {
        $$ = with_loc(std::make_unique<ast::UnOpNode>(ast::unop_node_type::logical_not, std::move($2)), @$, driver);
    }
    | MINUS expr %prec UMINUS
    {
        $$ = with_loc(std::make_unique<ast::UnOpNode>(ast::unop_node_type::neg, std::move($2)), @$, driver);
    }
    | PLUS expr %prec UMINUS
    {
        $$ = with_loc(std::make_unique<ast::UnOpNode>(ast::unop_node_type::pos, std::move($2)), @$, driver);
    }
    | LEFT_PAREN expr RIGHT_PAREN
    {
        $$ = std::move($2);
    }
    | NUMBER
    {
        $$ = with_loc(std::make_unique<ast::ValueNode>($1), @$, driver);
    }
    | VAR
    {
        $$ = with_loc(std::make_unique<ast::VarNode>(std::move($1)), @$, driver);
    }
    | lvalue ASSIGNMENT expr
    {
        if ($1->node_type() == ast::base_node_type::var) {
            auto var = static_cast<ast::VarNode*>($1.get());
            $$ = with_loc(std::make_unique<ast::VarDeclNode>(var->name(), std::move($3)), @$, driver);
        } else {
            $$ = with_loc(std::make_unique<ast::AssignNode>(std::move($1), std::move($3)), @$, driver);
        }
    }
    | QUESTION_MARK
    {
        $$ = with_loc(std::make_unique<ast::InputNode>(), @$, driver);
    }
    | lvalue ASSIGNMENT QUESTION_MARK
    {
        auto input = std::make_unique<ast::InputNode>(std::move($1));
        $$ = with_loc(std::move(input), @$, driver);
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