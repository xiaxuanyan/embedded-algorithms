cmake_minimum_required(VERSION 3.20)
project(embedded_algorithms C)

set(CMAKE_C_STANDARD 11)

# 环形缓冲区
add_executable(ring_buffer
        "UserCode/buffer/Ring buffer/Ring Buffer.c"
        "UserCode/buffer/Ring buffer/main.c"
)
target_include_directories(ring_buffer PRIVATE
        "${PROJECT_SOURCE_DIR}/UserCode/buffer/Ring buffer"
)

# 双缓冲区
add_executable(double_buffer
        "UserCode/buffer/Double_buffer/Double_buffer.c"
        "UserCode/buffer/Double_buffer/main.c"
)
target_include_directories(double_buffer PRIVATE
        "${PROJECT_SOURCE_DIR}/UserCode/buffer/Double_buffer"
)

# CRC16 测试程序
add_executable(crc16
        "UserCode/check/CRC16/crc16.c"
        "UserCode/check/CRC16/main.c"
)
target_include_directories(crc16 PRIVATE
        "${PROJECT_SOURCE_DIR}/UserCode/check/CRC16"
)

if(WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(crc16 PRIVATE -fexec-charset=GBK)
endif()