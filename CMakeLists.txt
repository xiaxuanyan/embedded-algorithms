cmake_minimum_required(VERSION 3.20)
project(embedded_algorithms C)

set(CMAKE_C_STANDARD 11)

# 环形缓冲区（生产者-消费者）演示，可运行入口 main.c
add_executable(ring_buffer
    "UserCode/buffer/Ring buffer/Ring Buffer.c"
    "UserCode/buffer/Ring buffer/Ring Buffer.h"
        "UserCode/buffer/Ring buffer/main.c"
        "UserCode/buffer/Ring buffer/main.h"
)

# 双缓冲区（乒乓缓冲）演示，可运行入口 main.c
add_executable(double_buffer
    "UserCode/buffer/Double_buffer/Double_buffer.c"
    "UserCode/buffer/Double_buffer/Double_buffer.h"
        "UserCode/buffer/Double_buffer/main.c"
)