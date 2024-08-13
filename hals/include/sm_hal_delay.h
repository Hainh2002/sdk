/*
 * sm_hal_delay.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef HAL_INCLUDE_SM_HAL_DELAY_H_
#define HAL_INCLUDE_SM_HAL_DELAY_H_

#include <stdint.h>
#include <sm_config.h>
#include <sm_hal_system.h>

#if !RTOS

/**
 * @fn void sm_hal_delay_ms(uint32_t)
 * @brief
 *
 * @param timems
 */
void sm_hal_delay_ms(uint32_t _timems);
/**
 * @fn void sm_hal_delay_us(uint32_t)
 * @brief
 *
 * @param timeus
 */
void sm_hal_delay_us(uint32_t _timeus);
/**
 * @fn uint32_t get_tick_count()
 * @brief
 *
 * @return
 */
//uint32_t get_tick_count() WEAK;

void sm_hal_system_clock_config() WEAK;

#else
/**
 * @fn void sm_hal_delay_ms(uint32_t)
 * @brief
 *
 * @param timems
 */
void sm_hal_delay_ms(uint32_t timems);
/**
 * @fn void sm_hal_delay_us(uint32_t)
 * @brief
 *
 * @param timeus
 */
void sm_hal_delay_us(uint32_t timeus);
/**
 * @fn uint32_t get_tick_count()
 * @brief
 *
 * @return
 */
uint32_t get_tick_count();
#endif


#endif /* hal_INCLUDE_SM_HAL_DELAY_H_ */
