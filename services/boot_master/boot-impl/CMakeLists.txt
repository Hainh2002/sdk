cmake_minimum_required(VERSION 3.22)
project(sm_boot_impl)

set(CMAKE_C_STANDARD 99)

add_library(sm_boot_impl STATIC
        sm_boot_impl.h
        input/sm_file_boot_input.c
        input/sm_modbus_rtu_boot_input.c
        output/sm_canopen_boot_output.c
        output/sm_file_boot_output.c
        output/sm_host_485_boot_output.c
        output/sm_modbus_rtu_boot_output.c
)

target_link_libraries(sm_boot_impl sdk_utils host co_impl)