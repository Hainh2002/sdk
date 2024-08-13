cmake_minimum_required(VERSION 3.22)
project(ev-boot_master)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(boot-core)
add_subdirectory(boot-impl)

add_library(sm_sv_boot_master STATIC
        sm_sv_boot_master.h
        sm_sv_boot_master.c)

target_link_libraries(sm_sv_boot_master mb_master sm_boot_core sm_boot_impl sdk_utils host)


add_executable(test_sv_boot example.c)
target_link_libraries(test_sv_boot sm_sv_boot_master sdk_utils -lpthread)

