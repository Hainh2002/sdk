cmake_minimum_required(VERSION 3.22)
project(testing_bp_management)

set(CMAKE_C_STANDARD 99)

add_executable(testing_bp_management
        testing_bp_management.c)

target_link_libraries(testing_bp_management sm_bp_management sm_core_sch)