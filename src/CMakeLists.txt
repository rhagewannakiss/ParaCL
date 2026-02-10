add_library(paracl_core INTERFACE)

target_include_directories(paracl_core
    INTERFACE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(paracl_core
    INTERFACE
        flags
)
