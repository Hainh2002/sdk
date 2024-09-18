cmake_minimum_required(VERSION 3.16)
project(sm_hals)
set(CMAKE_C_STANDARD 99)

add_library(sm_hals STATIC 
        sm_hal_io.h
        sm_hal_uart.h
        sm_hal_timer.h
        sm_hal_spi.h
        sm_hal.h
)

