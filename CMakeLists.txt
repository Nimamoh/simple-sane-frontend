cmake_minimum_required(VERSION 3.7)
project(sane_test)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)

add_executable(sane_test ${SOURCE_FILES})

find_library(SANE_LIB sane)
target_link_libraries(sane_test "${SANE_LIB}")

