//
// Created by DELL 5425 on 3/4/2024.
//

#ifndef EV_SDK_SM_HAL_WIN_H
#define EV_SDK_SM_HAL_WIN_H
#include <stdint.h>

typedef void win_io_ctrl_t;

typedef struct win_io_api {
    int32_t (* open_io)(win_io_ctrl_t * );
    int32_t (* close_io)(win_io_ctrl_t * );
    int32_t (* set_io)(win_io_ctrl_t * , uint8_t , uint8_t , uint8_t );
    int32_t (* get_io)(win_io_ctrl_t * , uint8_t , uint8_t , uint8_t *);
    int32_t (* cfg_io)(win_io_ctrl_t * , uint8_t , uint8_t , uint8_t );
} win_io_api_t ;

typedef struct win_io {
    win_io_ctrl_t * p_ctrl;
    win_io_api_t const * p_api;
} win_io_t ;

int32_t WIN_HAL_IO_OPEN(win_io_ctrl_t * _ctrl);
int32_t WIN_HAL_IO_CLOSE(win_io_ctrl_t * _ctrl);
int32_t WIN_HAL_IO_GET_VALUE(win_io_ctrl_t * , uint8_t _port, uint8_t _pin, uint8_t *_value);
int32_t WIN_HAL_IO_SET_VALUE(win_io_ctrl_t * , uint8_t _port, uint8_t _pin, uint8_t _value);
int32_t WIN_HAL_IO_CONFIG(win_io_ctrl_t * , uint8_t _port, uint8_t _pin, uint8_t _mode);
#endif //EV_SDK_SM_HAL_WIN_H

