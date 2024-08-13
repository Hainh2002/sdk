cmake_minimum_required(VERSION 3.16)
project(linux-platform)

set(CMAKE_C_STANDARD 99)

set(UNIX_UTILS_SRC
        utils/linux_serial.c
        utils/linux_serial.h
        utils/sm_datetime_unix.c
        utils/tcp_client.c
        utils/tcp_client.h
        utils/tcp_server.c
        utils/tcp_server.h
        sm_bsp_sys_linux.h
)

add_library(linux_utils STATIC
        ${UNIX_UTILS_SRC})

add_library(linux_platform sm_bsp_sys_linux.c
        sm_bsp_sys_linux.h)

target_link_libraries(linux_platform sdk_utils)