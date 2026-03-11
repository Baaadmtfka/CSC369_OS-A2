cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(
    csc369h1_memory_management
    VERSION 20241
    LANGUAGES C
)

set(CSC369H1_MEMORY_BIN_DIR ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(tests)
