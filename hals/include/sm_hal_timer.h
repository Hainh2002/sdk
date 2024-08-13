/*
 * sm_hal_timer.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_TIMER_H_
#define SM_HAL_TIMER_H_

#include <stdint.h>
#include <stdio.h>

#define SM_TIMER_STATIC_SIZE 80

typedef void (*timerfuntion_t)(void *arg);

typedef struct sm_hal_timer_static sm_hal_timer_static_t;

struct sm_hal_timer_static{
	uint8_t buff[SM_TIMER_STATIC_SIZE];
};

typedef struct sm_hal_timer sm_hal_timer_t;
/**
 * @struct sm_hal_timer_proc
 * @brief
 *
 */
typedef struct sm_hal_timer_proc
{
    /* data */
    int32_t (*start)(sm_hal_timer_t *_this);
    int32_t (*stop)(sm_hal_timer_t *_this);
    int32_t (*set_perios)(sm_hal_timer_t *_this, uint32_t _period);
    int32_t (*open)(sm_hal_timer_t *_this);
    int32_t (*close)(sm_hal_timer_t *_this);
} sm_hal_timer_proc_t;
/**
 * @struct sm_hal_timer
 * @brief
 *
 */
struct sm_hal_timer
{
    /* data */
    const sm_hal_timer_proc_t *proc;
    timerfuntion_t funtion;
    void *handle;
};
/**
 * @fn sm_hal_timer_t sm_hal_timer_init*(sm_hal_timer_proc_t*, timerfuntion_t, const char*, void*)
 * @brief
 *
 * @param m_proc
 * @param funtion
 * @param name
 * @param arg
 * @return
 */
sm_hal_timer_t* sm_hal_timer_init(sm_hal_timer_proc_t *_proc, timerfuntion_t _funtion, const char *_name, void *_arg,
        void *_handle);

sm_hal_timer_t* sm_hal_timer_init_static(sm_hal_timer_proc_t *_proc, timerfuntion_t _funtion, const char *_name, void *_arg,
        void *_handle,sm_hal_timer_static_t *buff);

/**
 * @fn void sm_hal_timer_deinit(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_timer_deinit(sm_hal_timer_t *_this);
/**
 * @fn void sm_hal_timer_callback(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_timer_callback(sm_hal_timer_t *_this); // Calling funtion in timer interrupt
/**
 * @fn void sm_hal_timer_set_callback(sm_hal_timer_t*, timerfuntion_t, void*)
 * @brief
 *
 * @param _this
 * @param funtion
 * @param arg
 */
void sm_hal_timer_set_callback(sm_hal_timer_t *_this, timerfuntion_t funtion, void *arg);
/**
 * @fn int32_t sm_hal_timer_start(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_timer_start(sm_hal_timer_t *_this)
{
    return _this->proc->start (_this);
}
/**
 * @fn int32_t sm_hal_timer_stop(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_timer_stop(sm_hal_timer_t *_this)
{
    return _this->proc->stop (_this);
}
/**
 * @fn int32_t sm_hal_timer_set_perios(sm_hal_timer_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param perios
 * @return
 */
static inline int32_t sm_hal_timer_set_period(sm_hal_timer_t *_this, uint32_t _period)
{
    return _this->proc->set_perios (_this, _period);
}
/**
 * @fn int32_t sm_hal_timer_open(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_timer_open(sm_hal_timer_t *_this)
{
    return _this->proc->open (_this);
}
/**
 * @fn int32_t sm_hal_timer_close(sm_hal_timer_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_timer_close(sm_hal_timer_t *_this)
{
    return _this->proc->close (_this);
}

#endif /* hal_INCLUDE_SM_HAL_TIMER_H_ */
