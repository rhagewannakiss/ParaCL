cmake_minimum_required(VERSION 3.21)

project(ParaCL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(flags)
add_subdirectory(src)

enable_testing()

add_subdirectory(test)

file(GLOB_RECURSE ALL_SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_SOURCE_DIR}/test/*.cpp"
    "${CMAKE_SOURCE_DIR}/test/*.hpp"
)

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(check-format
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Сlang-format code style check"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Clang-format code formating"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
    message(STATUS "clang-format not found, targets discluded")
endif()

find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK}
            --project=${CMAKE_BINARY_DIR}/compile_commands.json
            --enable=all
            --inconclusive
            --suppress=missingIncludeSystem
            --suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppress
            -i ${CMAKE_BINARY_DIR}/_deps
            -i ${CMAKE_BINARY_DIR}/src/grammar.tab.cc
            -i ${CMAKE_BINARY_DIR}/src/lex.yy.cc
            --error-exitcode=1
        COMMENT "Cppcheck run"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    add_dependencies(cppcheck parser_lib)
else()
    message(STATUS "Cppcheck not found, targets discluded")
endif()
