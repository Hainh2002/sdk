cmake_minimum_required(VERSION 3.22)
project(btn)

set(CMAKE_C_STANDARD 99)

add_executable(btn
        btn_testing.c
        sm_sv_bt.h
        sm_sv_bt.c)

target_link_libraries(btn sdk_utils)
