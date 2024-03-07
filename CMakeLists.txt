cmake_minimum_required(VERSION 3.28)
project(simple_shell)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(simple_shell
        README.md
        shell.c)
