cmake_minimum_required(VERSION 4.2)
project(OperationSysC C)

set(CMAKE_C_STANDARD 23)

add_executable(OperationSysC main.c
        parent.c
        child.c)
