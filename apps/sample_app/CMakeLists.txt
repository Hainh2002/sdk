cmake_minimum_required(VERSION 3.16)
project(sample_app)

set(CMAKE_C_STANDARD 99)

add_executable(sample_app
        sample_app.c
        sm_bsp.h)

target_link_libraries(sample_app sm_core_sch linux_platform)
