
/*
 * sm_hal_uart.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_UART_H_
#define SM_HAL_UART_H_

#include <stdint.h>

/**
 * @enum
 * @brief
 *
 */
typedef enum{
    SM_UART_PARITY_NONE = 0,/**< SM_UART_PARITY_NONE */
    SM_UART_PARITY_ODD, /**< SM_UART_PARITY_ODD */
    SM_UART_PARITY_EVEN, /**< SM_UART_PARITY_EVEN */
}SM_UART_PARITY;

/**
 * @enum
 * @brief
 *
 */
typedef enum{
    SM_UART_DATA_BIT7 = 0,    /**< SM_UART_DATA_BIT7 */
    SM_UART_DATA_BIT8,        /**< SM_UART_DATA_BIT8 */
    SM_UART_DATA_BIT9         /**< SM_UART_DATA_BIT9 */
}SM_UART_DATA_BIT;

typedef void sm_hal_uart_t;

typedef void (*sm_hal_uart_rx_irq_fn_t)(sm_hal_uart_t*, uint8_t, void*);
typedef void (*sm_hal_uart_tx_irq_fn_t)(sm_hal_uart_t*, void*);

/**
 * @fn sm_hal_uart_t sm_hal_uart_init*(sm_hal_uart_proc_t*, void*)
 * @brief
 *
 * @param fun
 * @param handle
 * @return
 */
sm_hal_uart_t* sm_hal_uart_init(const void* _channel, uint32_t _baud, uint8_t _stop_bit, uint8_t _data_bit);

/**
 * @fn void sm_hal_uart_deinit(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_uart_deinit(sm_hal_uart_t *_this);

/**
 * @fn int32_t sm_hal_uart_config
 * @brief
 * @param _this
 * @param _baud
 * @param _stop_bit
 * @param _data_bit
 * @return
 */
int32_t sm_hal_uart_config(sm_hal_uart_t *_this, uint32_t _baud, uint8_t _stop_bit, uint8_t _data_bit);

/**
 * @fn int32_t sm_hal_uart_write(sm_hal_uart_t*, uint8_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
int32_t sm_hal_uart_write(sm_hal_uart_t *_this, uint8_t *_buff, uint32_t _len);

/**
 * @fn int32_t sm_hal_uart_read(sm_hal_uart_t*, uint8_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
int32_t sm_hal_uart_read(sm_hal_uart_t *_this, uint8_t *_buff, uint32_t _len);

/**
 * @fn void sm_hal_uart_set_rx_callback(sm_hal_uart_t*, sm_hal_uart_rx_irq)
 * @brief
 *
 * @param _this
 * @param irq
 */
void sm_hal_uart_set_rx_cb(sm_hal_uart_t *_this, sm_hal_uart_rx_irq_fn_t _cb, void* _arg);

/**
 * @fn void sm_hal_uart_set_tx_callback(sm_hal_uart_t*, sm_hal_uart_tx_irq)
 * @brief
 *
 * @param _this
 * @param irq
 */
void sm_hal_uart_set_tx_cb(sm_hal_uart_t *_this, sm_hal_uart_tx_irq_fn_t _cb, void* _arg);

/**
 * @fn int32_t sm_hal_uart_open(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_uart_open(sm_hal_uart_t *_this);

/**
 * @fn int32_t sm_hal_uart_close(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_uart_close(sm_hal_uart_t *_this);

/**
 * @brief sm_hal_uart_irq: Call in interrupt UART
 * @param _this
 * @param _data
 * @return
 */
int32_t sm_hal_uart_rx_irq(sm_hal_uart_t* _this, uint8_t _data);

/**
 * @brief sm_hal_uart_irq: Call in interrupt UART
 * @param _this
 * @param _data
 * @return
 */
int32_t sm_hal_uart_tx_irq(sm_hal_uart_t* _this, uint8_t _data);

#endif /* SM_HAL_UART_H_ */
