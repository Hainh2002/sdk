cmake_minimum_required(VERSION 3.22)
project(pms)

set(CMAKE_C_STANDARD 99)

add_subdirectory(testing)

add_library(sm_sv_pms STATIC
        sm_sv_pms.c
        sm_sv_pms.h
        sm_pms_est_data.h
        sm_pms_data.h
        sm_pms_est.c)

target_link_libraries(sm_sv_pms sdk_utils)
target_link_libraries(sm_sv_pms sm_bp_management)
#target_link_libraries(sm_sv_pms sm_sv_charger)