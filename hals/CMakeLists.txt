cmake_minimum_required(VERSION 3.22)
project(sm_hals)
set(CMAKE_C_STANDARD 99)

set(SM_HALS_SRC
        src/sm_hal_adc.c
        src/sm_hal_can.c
#        src/sm_hal_delay.c
        src/sm_hal_flash.c
        src/sm_hal_iic.c
        src/sm_hal_io.c
        src/sm_hal_iwdg.c
        src/sm_hal_pwm.c
        src/sm_hal_spi.c
        src/sm_hal_system.c
        src/sm_hal_timer.c
        src/sm_hal_uart.c
        include/sm_hal_adc.h
        include/sm_hal_can.h
#        include/sm_hal_delay.h
        include/sm_hal_flash.h
        include/sm_hal_iic.h
        include/sm_hal_io.h
        include/sm_hal_iwdg.h
        include/sm_hal_pwm.h
        include/sm_hal_spi.h
        include/sm_hal_system.h
        include/sm_hal_timer.h
        include/sm_hal_uart.h
)
add_library(sm_hals STATIC ${SM_HALS_SRC})
#target_link_libraries()