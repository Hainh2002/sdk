
/*
 * sm_hal_timer.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_TIMER_H_
#define SM_HAL_TIMER_H_

#include <stdint.h>

enum {
    SM_HAL_TIMER_0,
    SM_HAL_TIMER_1,
    SM_HAL_TIMER_2,
    SM_HAL_TIMER_NUMBER
}SM_HAL_TIMER;

typedef void* sm_hal_timer_t;
typedef void (*sm_hal_timer_cb_fn_t)(uint8_t, void*);

/**
 * @fn sm_hal_timer_t sm_hal_timer_init()
 * @brief
 *@param _timer
 * @param _period
 * @return
 */
sm_hal_timer_t* sm_hal_timer_init(uint8_t _timer, uint32_t _period);

/**
 * @fn void sm_hal_timer_deinit(sm_hal_timer_t*)
 * @brief
 * @param _this
 */
void sm_hal_timer_deinit(sm_hal_timer_t *_this);

/**
 * @fn int32_t sm_hal_timer_set_period
 * @brief
 * @param _this
 * @param _period
 * @return
 */
int32_t sm_hal_timer_set_period(sm_hal_timer_t *_this, uint32_t _period);

/**
 * @fn void sm_hal_timer_set_callback()
 * @brief
 * @param _this
 * @param _cb
 * @param arg
 */
void sm_hal_timer_set_callback(sm_hal_timer_t *_this, sm_hal_timer_cb_fn_t _cb, void *arg);

/**
 * @fn int32_t sm_hal_timer_start(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_timer_start(sm_hal_timer_t *_this);

/**
 * @fn int32_t sm_hal_timer_stop(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_timer_stop(sm_hal_timer_t *_this);

/**
 * @fn void sm_hal_timer_callback(sm_hal_timer_t*)
 * @brief
 * @param _this
 */
void sm_hal_timer_irq(sm_hal_timer_t *_this); // Calling function in timer interrupt

#endif /* SM_HAL_TIMER_H_ */
