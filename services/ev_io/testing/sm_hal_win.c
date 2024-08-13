//
// Created by DELL 5425 on 3/4/2024.
//
#include "sm_hal_win.h"
#include <stdio.h>

const win_io_api_t g_io = {
        .open_io = WIN_HAL_IO_OPEN,
        .close_io = WIN_HAL_IO_CLOSE,
        .get_io = WIN_HAL_IO_GET_VALUE,
        .set_io = WIN_HAL_IO_SET_VALUE,
        .cfg_io = WIN_HAL_IO_CONFIG,
};
static uint8_t port_pin_value[16][16];
static uint8_t port_pin_mode[16][16];

int32_t WIN_HAL_IO_OPEN(win_io_ctrl_t * _ctrl){
    (void*) _ctrl;
    printf("WIN_HAL_IO_OPEN SUCCESS\n");
    for (uint8_t i=0; i<16; i++)
        for (uint8_t j=0; j<16; j++){
            port_pin_value[i][j] = 0;
            port_pin_mode[i][j] = 0;
        }
    return 0;
}
int32_t WIN_HAL_IO_CLOSE(win_io_ctrl_t * _ctrl){
    (void*) _ctrl;
    printf("WIN_HAL_IO_CLOSE SUCCESS\n");
    for (uint8_t i=0; i<16; i++)
        for (uint8_t j=0; j<16; j++){
            port_pin_value[i][j] = 0;
            port_pin_mode[i][j] = 0;
        }
    return 0;
}
int32_t WIN_HAL_IO_GET_VALUE(win_io_ctrl_t * _ctrl, uint8_t _port, uint8_t _pin, uint8_t *_value){
    (void*) _ctrl;
    uint8_t temp_value = port_pin_value[_port][_pin];
    printf("WIN_HAL_IO_GET_VALUE SUCCESS\n");
    printf("PORT %d PIN %d | %d\n", _port, _pin, temp_value);
    *_value = temp_value;
    return 0;
}
int32_t WIN_HAL_IO_SET_VALUE(win_io_ctrl_t * _ctrl, uint8_t _port, uint8_t _pin, uint8_t _value){
    (void*) _ctrl;
    port_pin_value[_port][_pin] = _value;
    printf("WIN_HAL_IO_SET_VALUE SUCCESS\n");
    printf("PORT %d PIN %d | %d\n", _port, _pin, _value);
    return 0;
}
int32_t WIN_HAL_IO_CONFIG(win_io_ctrl_t * _ctrl, uint8_t _port, uint8_t _pin, uint8_t _mode){
    (void*) _ctrl;
    port_pin_mode[_port][_pin] = _mode;
    printf("WIN_HAL_IO_CONFIG SUCCESS\n");
    printf("PORT %d PIN %d | %d\n", _port, _pin, _mode);
    return 0;
}