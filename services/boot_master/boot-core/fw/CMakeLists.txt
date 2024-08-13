cmake_minimum_required(VERSION 3.22)
project(sm_fw)

set(CMAKE_C_STANDARD 99)

add_library(sm_fw STATIC
        sm_fw.c sm_fw.h sm_segment.c sm_segment.h)

target_link_libraries(sm_fw sdk_utils)