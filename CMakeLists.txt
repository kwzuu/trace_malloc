cmake_minimum_required(VERSION 3.24)
project(trace_malloc C)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "-Wall -Werror -Wextra -Wshadow -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-g3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_executable(trace_malloc main.c trace_malloc.c hash_table.c hash_table.h)
