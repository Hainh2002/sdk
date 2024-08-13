//
// Created by DELL 5425 on 3/4/2024.
//

#ifndef EV_SDK_SM_BSP_WIN_H
#define EV_SDK_SM_BSP_WIN_H

#include "sm_hal_win.h"

extern const win_io_t g_win_io;
//extern sm_hal_io_proc_t io_func;

void logger_put(const char* log);
int32_t sm_bsp_init();
uint8_t sm_bsp_io_get_value_key();
uint8_t sm_bsp_io_get_value_left_light();
uint8_t sm_bsp_io_get_value_right_light();
uint8_t sm_bsp_io_get_value_phare_light();
uint8_t sm_bsp_io_get_value_cos_light();
uint8_t sm_bsp_io_get_value_parking();
uint8_t sm_bsp_io_get_value_driving_mode();
uint8_t sm_bsp_io_get_value_reverse_mode();
uint8_t sm_bsp_io_get_value_left_break();
uint8_t sm_bsp_io_get_value_right_break();
uint8_t sm_bsp_io_get_value_horn();

int32_t sm_bsp_io_set_value_left_light(uint8_t _value);
int32_t sm_bsp_io_set_value_right_light(uint8_t _value);
int32_t sm_bsp_io_set_value_phare_light(uint8_t _value);
int32_t sm_bsp_io_set_value_cos_light(uint8_t _value);
int32_t sm_bsp_io_set_value_tail_light(uint8_t _value);
int32_t sm_bsp_io_set_value_horn(uint8_t _value);


#endif //EV_SDK_SM_BSP_WIN_H
