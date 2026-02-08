cmake_minimum_required(VERSION 3.21)

project(ParaCL)

if (GRAPHVIZ)
    add_definitions(-DGRAPHVIZ=1)
    message(STATUS "graphviz enabled")
else()
    message(STATUS "graphviz disabled")
endif()

add_subdirectory(flags)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/
        src/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        flags
)