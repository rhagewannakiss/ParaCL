include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(ast_clone_test
    ast_clone_test.cpp
)

target_link_libraries(ast_clone_test
    PRIVATE
        paracl_core
        GTest::gtest_main
)

add_executable(ast_print_test
    ast_print_test.cpp
)

target_link_libraries(ast_print_test
    PRIVATE
        paracl_core
)

add_executable(ast_nodes_test
    ast_nodes_test.cpp
)

target_link_libraries(ast_nodes_test
    PRIVATE
        paracl_core
)

include(GoogleTest)
gtest_discover_tests(ast_clone_test)
