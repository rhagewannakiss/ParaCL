#include "Visitors/Interpreter.hpp"
#include "Visitors/SemanticChecker.hpp"
#include "driver/driver.hpp"
#include "errors-output/error-formatter.hpp"

#include <FlexLexer.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int parse_and_run(const char* path)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        std::cerr << err::format_error(ast::SourceRange(),
                                       "Failed to open file: " +
                                           std::string(path))
                  << '\n';
        return 1;
    }

    yyFlexLexer lexer(&input);
    yy::NumDriver driver(&lexer, path);

    try {
        if (!driver.parse())
            return 1;
        if (driver.has_errors())
            return 1;

        ast::AST ast_tree = driver.get_ast();
        if (ast_tree.root() == nullptr) {
            std::cerr << err::format_error(ast::SourceRange(),
                                           "Parser produced empty AST")
                      << '\n';
            return 1;
        }

        ast::SemanticChecker checker;
        checker.check(ast_tree.root());
        if (checker.hasErrors()) {
            checker.printErrors(std::cerr);
            return 1;
        }

        ast::Interpreter interpreter;
        ast_tree.root()->accept(interpreter);
    } catch (const std::runtime_error& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    return parse_and_run(argv[1]);
}