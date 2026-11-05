cmake_minimum_required(VERSION 3.13)
project(rv32s C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB_RECURSE SRC CMAKE_CONFIGURE_DEPENDS "src/*.c")
add_executable(program ${SRC})
target_include_directories(program PRIVATE include)
