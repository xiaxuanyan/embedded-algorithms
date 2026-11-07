cmake_minimum_required(VERSION 3.20)
project(embedded_algorithms C)

set(CMAKE_C_STANDARD 11)

add_executable(ring_buffer
    "UserCode/buffer/Ring buffer/Ring Buffer.c"
    "UserCode/buffer/Ring buffer/Ring Buffer.h"
        "UserCode/buffer/Ring buffer/main.c"
        "UserCode/buffer/Ring buffer/main.h"
)