cmake_minimum_required(VERSION 3.21)

project(ParaCL)

add_subdirectory(flags)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        flags
)

# enable_testing()
# add_subdirectory(tests)