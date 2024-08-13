cmake_minimum_required(VERSION 3.22)
project(sm_boot_core)

set(CMAKE_C_STANDARD 99)

add_library(sm_boot_core STATIC
        sm_boot_master.c sm_boot_master.h sm_boot_slave.c sm_boot_slave.h)

target_link_libraries(sm_boot_core sdk_utils sm_fw)

add_subdirectory(fw)