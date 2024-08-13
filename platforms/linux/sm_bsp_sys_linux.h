//
// Created by vnbk on 24/05/2024.
//

#ifndef EV_SDK_SM_BSP_SYS_LINUX_H
#define EV_SDK_SM_BSP_SYS_LINUX_H

#include <stdint.h>

extern int32_t get_tick_count();
void sm_bsp_linux_logger_put(const char* log);

#endif //EV_SDK_SM_BSP_SYS_LINUX_H
