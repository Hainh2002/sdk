cmake_minimum_required(VERSION 3.22)
project(co_impl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(CO_SERIAL_INTERFACE OFF)
set(CO_ETHERNET_INTERFACE ON)

include_directories(canopen/
        od/
        od/comm_od/
        od/manu_od/
        interface/
        init/)

if (CO_SERIAL_INTERFACE)
    set(CO_INTERFACE_SRC
            interface/sm_co_serial_if.c
            interface/sm_co_if.h)
endif ()

if (CO_ETHERNET_INTERFACE)
    set(CO_INTERFACE_SRC
            interface/sm_co_ethernet_if.c
            interface/sm_co_if.h)
endif ()

set(CO_IMPL_SRC
        ${CO_INTERFACE_SRC}
        od/od.h
        od/od.c
        od/od_config.h
        od/manu_od/manu_od_default_value.c
        od/manu_od/manu_od.c
        od/comm_od/comm_od_default_value.c
        od/comm_od/comm_od.c
        init/app_co_init.c
        init/app_co_init.h
)

add_library(co_impl STATIC
        ${CO_IMPL_SRC})

target_link_libraries(co_impl co sdk_unix_utils sdk_utils)

if (CO_SERIAL_INTERFACE)
    target_link_libraries(co_impl host)
endif ()