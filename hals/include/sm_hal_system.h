/*
 * sm_sbp_system.h
 *
 *  Created on: Aug 28, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_SYSTEM_H_
#define SM_HAL_SYSTEM_H_

#include <stdint.h>


#define WEAK        __attribute__((weak))


void sm_hal_reset() WEAK;
void sm_hal_jump_addr(uint32_t addr) WEAK;

#endif /* hal_INCLUDE_SM_HAL_SYSTEM_H_ */
