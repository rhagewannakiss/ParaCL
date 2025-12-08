cmake_minimum_required(VERSION 3.21)

project(ParaCL)

add_subdirectory(flags)
add_subdirectory(src)

enable_testing()
add_subdirectory(test)
