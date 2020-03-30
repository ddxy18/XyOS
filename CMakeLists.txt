cmake_minimum_required(VERSION 3.14)
project(XyOS C)

set(CMAKE_C_STANDARD 11)

add_executable(XyOS kernel/interrupt/interrupt.c kernel kernel/start.c kernel/driver/console.c kernel/driver/console.h kernel/interrupt/trap.c kernel/interrupt/trap.h kernel/start.h kernel/test/test.c kernel/test/test.h)