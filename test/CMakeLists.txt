find_package(GTest REQUIRED)
include(GoogleTest)

function(add_paracl_unittest source_file)
    set(options USE_PARSER_LIB)
    cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

    get_filename_component(test_name "${source_file}" NAME_WE)

    add_executable("${test_name}" "${source_file}")

    if(ARG_USE_PARSER_LIB)
        target_link_libraries("${test_name}"
            PRIVATE
                parser_lib
                GTest::gtest_main
        )
    else()
        target_link_libraries("${test_name}"
            PRIVATE
                paracl_core
                GTest::gtest_main
        )
    endif()

    add_paracl_compile_options("${test_name}")
    gtest_discover_tests("${test_name}")
endfunction()

set(PARACL_CORE_TESTS
    AST_tests/ast_clone_test.cpp
    AST_tests/ast_print_test.cpp
    AST_tests/ast_nodes_test.cpp

    Visitor_tests/interpreter_runtime_validation_test.cpp
    Visitor_tests/interpreter_expr_test.cpp
    Visitor_tests/interpreter_expr_errors_test.cpp
    Visitor_tests/interpreter_short_circuit_test.cpp
    Visitor_tests/interpreter_control_flow_test.cpp
    Visitor_tests/interpreter_stmt_test.cpp
    Visitor_tests/dot_visitor_test.cpp
)

set(PARACL_PARSER_TESTS
    Parser_tests/lexer_test.cpp
    Parser_tests/parser_test.cpp
)

foreach(test_src IN LISTS PARACL_CORE_TESTS)
    add_paracl_unittest("${test_src}")
endforeach()

foreach(test_src IN LISTS PARACL_PARSER_TESTS)
    add_paracl_unittest("${test_src}" USE_PARSER_LIB)
endforeach()
