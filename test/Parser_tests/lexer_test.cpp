#include "grammar.tab.hh"

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <gtest/gtest.h>
#include <FlexLexer.h>

TEST(LexerTest, BasicTokens) {
    std::stringstream input("x = 5;");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_STREQ(lexer.YYText(), "x");
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::ASSIGNMENT);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NUMBER);
    EXPECT_STREQ(lexer.YYText(), "5");
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::SEMICOLON);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, ArithmeticOperators) {
    std::stringstream input("a + b - c * d / e % f;");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::PLUS);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::MINUS);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::MUL);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::DIV);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::MODULUS);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::SEMICOLON);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, ComparisonOperators) {
    std::stringstream input("x < y > z <= w >= v == u != t;");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::LESS);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::GREATER);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::LESS_OR_EQUAL);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::GREATER_OR_EQUAL);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::EQUAL);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NOT_EQUAL);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::SEMICOLON);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, LogicalOperators) {
    std::stringstream input("a && b || c ! d ^ e;");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::AND);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::OR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NOT);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::XOR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::SEMICOLON);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, BracketsAndDelimiters) {
    std::stringstream input("( ) { } , ?");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::LEFT_PAREN);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::RIGHT_PAREN);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::LEFT_CURLY_BRACKET);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::RIGHT_CURLY_BRACKET);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::COMMA);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::QUESTION_MARK);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, Keywords) {
    std::stringstream input("if else while for print");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::IF);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::ELSE);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::WHILE);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::FOR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::PRINT);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, VariablesAndNumbers) {
    std::stringstream input("var123 _var 123 0");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_STREQ(lexer.YYText(), "var123");
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_STREQ(lexer.YYText(), "_var");
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NUMBER);
    EXPECT_STREQ(lexer.YYText(), "123");
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NUMBER);
    EXPECT_STREQ(lexer.YYText(), "0");
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, CommentsAndWhitespace) {
    std::stringstream input("// comment\n x\t=  42;");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);  // x (skips comment and ws)
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::ASSIGNMENT);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NUMBER);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::SEMICOLON);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, InvalidCharacters) {
    std::stringstream input("@invalid$");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::ERR);  // @
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);  // invalid
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::ERR);  // $
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, MultiLineInput) {
    std::stringstream input("x = 1;\ny = 2;");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::ASSIGNMENT);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NUMBER);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::SEMICOLON);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NEWLINE);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::ASSIGNMENT);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NUMBER);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::SEMICOLON);
    EXPECT_EQ(lexer.yylex(), 0);
}

TEST(LexerTest, EdgeCases) {
    std::stringstream input("long_var_123 999999 //end\n?");
    yyFlexLexer lexer(&input);

    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::VAR);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::NUMBER);
    EXPECT_EQ(lexer.yylex(), yy::parser::token_type::QUESTION_MARK);
    EXPECT_EQ(lexer.yylex(), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}