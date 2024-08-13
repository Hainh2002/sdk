//
// Created by DELL 5425 on 3/4/2024.
//
#include "../hals/io/sm_hal_io.h"
#include "stdio.h"
#include "sm_hal_win.h"
#define _impl(x) (win_io_t*)(x)

static int32_t gpio_open(sm_hal_io_t *, uint8_t , uint8_t , uint8_t);
static int32_t gpio_close(sm_hal_io_t *);
static int32_t gpio_set_value(sm_hal_io_t*, uint8_t);
static uint8_t gpio_get_value(sm_hal_io_t*);

sm_hal_io_proc_t io_func = {
        .open_fn_t = gpio_open,
        .close_fn_t = gpio_close,
        .set_value_fn_t = gpio_set_value,
        .get_value_fn_t = gpio_get_value,
};

static int32_t gpio_open(sm_hal_io_t *_this, uint8_t _port, uint8_t _pin, uint8_t _mode){
    win_io_t* p_io = _impl(_this)->m_handle;
    _this->m_mode = _mode;
    return WIN_HAL_IO_OPEN(p_io->p_ctrl);
}
static int32_t gpio_close(sm_hal_io_t *_this){
    win_io_t* p_io = _impl(_this)->m_handle;
    return WIN_HAL_IO_CLOSE(p_io->p_ctrl);
}
static int32_t gpio_set_value(sm_hal_io_t* _this, uint8_t _value){
    win_io_t* p_io = _impl(_this)->m_handle;
    return WIN_HAL_IO_SET_VALUE(p_io->p_ctrl, _this->m_port, _this->m_pin, _value);
}
static uint8_t gpio_get_value(sm_hal_io_t *_this){
    win_io_t* p_io = _impl(_this)->m_handle;
    uint8_t value;
    WIN_HAL_IO_GET_VALUE(p_io->p_ctrl, _this->m_port, _this->m_pin, &value);
    return value;
}