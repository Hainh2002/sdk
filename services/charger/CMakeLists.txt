cmake_minimum_required(VERSION 3.22)
project(charger)

set(CMAKE_C_STANDARD 99)

add_subdirectory(testing)

add_library(sm_sv_charger STATIC
        sm_sv_charger.c
        sm_sv_charger.h
)

target_link_libraries(sm_sv_charger sm_bp_management sdk_utils)