/*
 * sm_hal_adc.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef INCLUDE_SM_HAL_ADC_H_
#define INCLUDE_SM_HAL_ADC_H_

#include <stdint.h>
#include <stdio.h>

typedef struct sm_hal_adc sm_hal_adc_t;

/**@brief adc function interface
 * @param start
 * @param stop
 * @param open
 * @param close
 */
typedef struct
{
    /* data */
    int32_t (*start)(sm_hal_adc_t *_this);
    int32_t (*stop)(sm_hal_adc_t *_this);
    int32_t (*open)(sm_hal_adc_t *_this);
    int32_t (*close)(sm_hal_adc_t *_this);
    int32_t (*read)(sm_hal_adc_t *_this, uint8_t _channel);
} sm_hal_adc_proc_t;

/**
 * @brief adc driver
 * @param proc
 * @param handle
 */
struct sm_hal_adc
{
    const sm_hal_adc_proc_t *proc;
    void *handle;
};

/**
 * @brief sm_hal_adc_init
 * @param _proc
 * @param _handle
 * @return *sm_hal_adc_t
 */

sm_hal_adc_t* sm_hal_adc_init(sm_hal_adc_proc_t *_proc, void *_handle, uint8_t _num_channel);

/**
 * @brief sm_hal_adc_deinit
 * @param _this
 */

void sm_hal_adc_deinit(sm_hal_adc_t *_this);

/**
 * @brief sm_hal_adc_read
 * @param _this
 * @return
 */

uint16_t sm_hal_adc_read(sm_hal_adc_t *_this, uint8_t _channel);

/**
 * @brief sm_hal_adc_start
 * @param _this
 * @return
 */

static inline int32_t sm_hal_adc_start(sm_hal_adc_t *_this)
{
    return _this->proc->start (_this);
}

/**
 * @brief
 * sm_hal_adc_stop
 * @param _this
 * @return
 */

static inline int32_t sm_hal_adc_stop(sm_hal_adc_t *_this)
{
    return _this->proc->stop (_this);
}

/**
 * @brief sm_hal_adc_callback
 * @param _this
 * @param _value
 */

void sm_hal_adc_callback(sm_hal_adc_t *_this, uint16_t _value);

/**
 * @brief sm_hal_adc_open
 * @param _this
 * @return 0
 */

int32_t sm_hal_adc_open(sm_hal_adc_t *_this);

/**
 * @brief sm_hal_adc_open
 * @param _this
 * @return 0
 */

int32_t sm_hal_adc_close(sm_hal_adc_t *_this);

#endif /* hal_INCLUDE_SM_HAL_ADC_H_ */
