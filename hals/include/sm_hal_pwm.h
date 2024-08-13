#ifndef INCLUDE_SM_HAL_PWM_H_
#define INCLUDE_SM_HAL_PWM_H_

#include <stdint.h>
#include <stdio.h>

typedef struct sm_hal_pwm sm_hal_pwm_t;
/**
 * @struct sm_hal_pwm_proc
 * @brief
 *
 */
typedef struct sm_hal_pwm_proc
{
    /* data */
    int32_t (*set_frequency)(sm_hal_pwm_t *_this, uint32_t _freq);
    int32_t (*set_dutycycles)(sm_hal_pwm_t *_this, uint32_t _dutycycle);
    int32_t (*start)(sm_hal_pwm_t *_this);
    int32_t (*stop)(sm_hal_pwm_t *_this);
    int32_t (*open)(sm_hal_pwm_t *_this);
    int32_t (*close)(sm_hal_pwm_t *_this);
} sm_hal_pwm_proc_t;

/**
 * @struct sm_hal_pwm
 * @brief
 *
 */
struct sm_hal_pwm
{
    /* data */
    void *handle;
    const sm_hal_pwm_proc_t *proc;
};
/**
 * @fn sm_hal_pwm_t sm_hal_pwm_init*(sm_hal_pwm_proc_t*, void*)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @return
 */
sm_hal_pwm_t* sm_hal_pwm_init(sm_hal_pwm_proc_t *m_proc, void *_handle);
/**
 * @fn void sm_hal_pwm_deinit(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_pwm_deinit(sm_hal_pwm_t *_this);
/**
 * @fn int32_t sm_hal_pwm_set_freq(sm_hal_pwm_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param freq
 * @return
 */
static inline int32_t sm_hal_pwm_set_freq(sm_hal_pwm_t *_this, uint32_t _freq)
{
    return _this->proc->set_frequency (_this, _freq);
}
/**
 * @fn int32_t sm_hal_pwm_set_dutycycles(sm_hal_pwm_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param dutycycle
 * @return
 */
static inline int32_t sm_hal_pwm_set_dutycycles(sm_hal_pwm_t *_this, uint32_t _dutycycle)
{
    return _this->proc->set_dutycycles (_this, _dutycycle);
}
/**
 * @fn int32_t sm_hal_pwm_set_start(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_pwm_set_start(sm_hal_pwm_t *_this)
{
    return _this->proc->start (_this);
}
/**
 * @fn int32_t sm_hal_pwm_set_stop(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_pwm_set_stop(sm_hal_pwm_t *_this)
{
    return _this->proc->stop (_this);
}
/**
 * @fn int32_t sm_hal_pwm_set_open(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_pwm_set_open(sm_hal_pwm_t *_this)
{
    return _this->proc->open (_this);
}
/**
 * @fn int32_t sm_hal_pwm_set_close(sm_hal_pwm_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_pwm_set_close(sm_hal_pwm_t *_this)
{
    return _this->proc->close (_this);
}
#endif
