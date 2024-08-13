cmake_minimum_required(VERSION 3.22)
project(canopen-service)

set(CMAKE_C_STANDARD 99)

add_library(sv_canopen STATIC sm_sv_co.c sm_sv_co.h sm_sv_co_define.h)

target_link_libraries(sv_canopen co_impl)