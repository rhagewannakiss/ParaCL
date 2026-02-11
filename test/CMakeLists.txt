include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(ast_clone_test
    AST_tests/ast_clone_test.cpp
)

target_link_libraries(ast_clone_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(ast_print_test
    AST_tests/ast_print_test.cpp
)

target_link_libraries(ast_print_test
    PRIVATE
        paracl_core
)

add_executable(ast_nodes_test
    AST_tests/ast_nodes_test.cpp
)

target_link_libraries(ast_nodes_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(interpreter_runtime_validation_test
    Visitor_tests/interpreter_runtime_validation_test.cpp
)

target_link_libraries(interpreter_runtime_validation_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(interpreter_expr_test
    Visitor_tests/interpreter_expr_test.cpp
)

target_link_libraries(interpreter_expr_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(interpreter_expr_errors_test
    Visitor_tests/interpreter_expr_errors_test.cpp
)

target_link_libraries(interpreter_expr_errors_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(interpreter_short_circuit_test
    Visitor_tests/interpreter_short_circuit_test.cpp
)

target_link_libraries(interpreter_short_circuit_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(interpreter_control_flow_test
    Visitor_tests/interpreter_control_flow_test.cpp
)

target_link_libraries(interpreter_control_flow_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(interpreter_stmt_test
    Visitor_tests/interpreter_stmt_test.cpp
)

target_link_libraries(interpreter_stmt_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ast_clone_test)
gtest_discover_tests(ast_nodes_test)
gtest_discover_tests(interpreter_runtime_validation_test)
gtest_discover_tests(interpreter_expr_test)
gtest_discover_tests(interpreter_expr_errors_test)
gtest_discover_tests(interpreter_short_circuit_test)
gtest_discover_tests(interpreter_control_flow_test)
gtest_discover_tests(interpreter_stmt_test)
