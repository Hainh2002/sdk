cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)
# Set project name
project(ev-io-testing)
#find_package(Curses REQUIRED)
#include_directories(${CURSES_INCLUDE_DIR})
add_executable(ev_io_testing
        testing.c
        sm_bsp_win.c
        sm_hal_win.c)



#target_link_libraries(ev_io_testing ${CURSES_LIBRARIES})
target_link_libraries(ev_io_testing sm_sv_ev_io)

