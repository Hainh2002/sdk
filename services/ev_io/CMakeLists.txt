cmake_minimum_required(VERSION 3.22)
project(ev-io)

set(CMAKE_C_STANDARD 99)

add_subdirectory(testing)

add_library(sm_sv_ev_io STATIC
        sm_sv_io.c)

target_link_libraries(sm_sv_ev_io sdk_utils)
