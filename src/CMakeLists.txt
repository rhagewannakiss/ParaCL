find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(Parser parser/grammar.y
        ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cc
    DEFINES_FILE
        ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.hh
    COMPILE_FLAGS
        "-Wno-conflicts-sr"
)

FLEX_TARGET(Lexer parser/grammar.l
        ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)


add_library(paracl_core
    STATIC
        interpeter/Interpreter.cpp
        interpeter/detail/ScopeGuard.cpp
        interpeter/detail/VarTable.cpp
)

target_include_directories(paracl_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(paracl_core
    PUBLIC
        flags
)

add_library(parser_generated
    OBJECT
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS}
)

target_include_directories(parser_generated
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FLEX_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(parser_generated
    PRIVATE
        -w)

add_library(parser_lib
    STATIC
        parser/scanner_location_bridge.cpp
)

target_include_directories(parser_lib
    SYSTEM PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FLEX_INCLUDE_DIRS}
)

target_link_libraries(parser_lib
    PUBLIC
        paracl_core
        $<TARGET_OBJECTS:parser_generated>
)

add_executable(paracl_cli
    main.cpp
)

set_target_properties(paracl_cli PROPERTIES
    OUTPUT_NAME paracl-cli
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(paracl_cli
    PRIVATE
        parser_lib
)
