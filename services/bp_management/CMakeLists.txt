cmake_minimum_required(VERSION 3.22)
project(bp)

set(CMAKE_C_STANDARD 99)

add_library(sm_bp_management STATIC
        sm_bp.h
        sm_bp_data.h
        sm_sv_bp.h
        sm_bp_auth.c
        sm_bp_auth.h
        sm_bp_co.c
        sm_bp_co.h
        sm_sv_bp.c
        sm_bp_co.h
        canopen/init/app_co_init.c
        canopen/od/od.c
        canopen/od/od.h
        canopen/od/od_config.h
        canopen/od/manu_od/manu_od.c
        canopen/od/manu_od/manu_od_default_value.c
        canopen/od/comm_od/comm_od.c
        canopen/od/comm_od/comm_od_default_value.c
        canopen/interface/sm_co_host_if.c
)

target_link_libraries(sm_bp_management sdk_utils co host)