find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(Parser parser/grammar.y
        ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cc
    DEFINES_FILE
        ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.hh)

FLEX_TARGET(Lexer parser/grammar.l
        ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)


add_library(paracl_core INTERFACE)

target_include_directories(paracl_core
    INTERFACE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(paracl_core
    INTERFACE
        flags
)

add_library(parser_lib
    STATIC
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Lexer_OUTPUTS}
)

target_include_directories(parser_lib
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FLEX_INCLUDE_DIR}
)

target_link_libraries(parser_lib
    PUBLIC
        paracl_core
)