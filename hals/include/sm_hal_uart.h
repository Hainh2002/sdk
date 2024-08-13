/*
 * sm_hal_uart.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_UART_H_
#define SM_HAL_UART_H_

#include <sm_fifo.h>
#include <stdint.h>
#include <stdio.h>
/**
 * @enum
 * @brief
 *
 */

typedef enum
{
    SM_UART_PARITY_NONE = 0,/**< SM_UART_PARITY_NONE */
    SM_UART_PARITY_ODD, /**< SM_UART_PARITY_ODD */
    SM_UART_PARITY_EVEN, /**< SM_UART_PARITY_EVEN */
} sm_uart_parity_t;
/**
 * @enum
 * @brief
 *
 */
typedef enum
{
    SM_UART_DATA_BIT7 = 0,/**< SM_UART_DATA_BIT7 */
    SM_UART_DATA_BIT8, /**< SM_UART_DATA_BIT8 */
    SM_UART_DATA_BIT9 /**< SM_UART_DATA_BIT9 */
} sm_uart_databit_t;

typedef struct sm_hal_uart sm_hal_uart_t;

typedef void (*sm_hal_uart_rx_irq)(sm_hal_uart_t *_this, uint8_t *_data, uint32_t _len);

typedef void (*sm_hal_uart_tx_irq)(sm_hal_uart_t *_this);
/**
 * @struct sm_hal_uart_proc
 * @brief
 *
 */
typedef struct sm_hal_uart_proc
{
    int32_t (*write)(sm_hal_uart_t *_this, uint8_t *_buff, uint32_t _len);
    int32_t (*set_baudrate)(sm_hal_uart_t *_this, uint32_t _baudrate);
    int32_t (*set_parity)(sm_hal_uart_t *_this, sm_uart_parity_t _parity);
    int32_t (*set_databit)(sm_hal_uart_t *_this, sm_uart_databit_t _databit);
    int32_t (*open)(sm_hal_uart_t *_this);
    int32_t (*close)(sm_hal_uart_t *_this);
    int32_t (*abort_rx)(sm_hal_uart_t *_this);
    int32_t (*abort_tx)(sm_hal_uart_t *_this);
} sm_hal_uart_proc_t;
/**
 * @struct sm_hal_uart
 * @brief
 *
 */
struct sm_hal_uart
{
    const sm_hal_uart_proc_t *proc;
    void *handle;
};

#define SM_UART_SIZE 80

typedef struct sm_uart_static sm_uart_static_t;



struct sm_uart_static{
	uint8_t buff[SM_UART_SIZE];
};

/**
 * @fn sm_hal_uart_t sm_hal_uart_init*(sm_hal_uart_proc_t*, void*)
 * @brief
 *
 * @param fun
 * @param handle
 * @return
 */
sm_hal_uart_t* sm_hal_uart_init(sm_hal_uart_proc_t *_fun, void *_handle);

sm_hal_uart_t* sm_hal_uart_init_static(sm_hal_uart_proc_t *_func,void *_handle,sm_uart_static_t *buff);

/**
 * @fn void sm_hal_uart_deinit(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_uart_deinit(sm_hal_uart_t *_this);
/**
 * @fn void sm_hal_uart_rx_callback(sm_hal_uart_t*, uint8_t*, uint16_t)
 * @brief
 *
 * @param _this
 * @param data
 * @param len
 */
void sm_hal_uart_rx_callback(sm_hal_uart_t *_this, uint8_t *_data, uint16_t _len);
/**
 * @fn void sm_hal_uart_tx_callback(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_uart_tx_callback(sm_hal_uart_t *_this);
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
size_t sm_hal_uart_read(sm_hal_uart_t *_this, uint8_t *_buff, uint32_t _len);
/**
 * @fn int32_t sm_hal_uart_peek(sm_hal_uart_t*, uint8_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
size_t sm_hal_uart_peek(sm_hal_uart_t *_this, uint8_t* _buff, uint32_t _len);
/**
 * @fn void sm_hal_uart_set_rx_callback(sm_hal_uart_t*, sm_hal_uart_rx_irq)
 * @brief
 *
 * @param _this
 * @param irq
 */
void sm_hal_uart_set_rx_callback(sm_hal_uart_t *_this, sm_hal_uart_rx_irq _irq);
/**
 * @fn void sm_hal_uart_set_tx_callback(sm_hal_uart_t*, sm_hal_uart_tx_irq)
 * @brief
 *
 * @param _this
 * @param irq
 */
void sm_hal_uart_set_tx_callback(sm_hal_uart_t *_this, sm_hal_uart_tx_irq _irq);
/**
 * @fn int32_t sm_hal_uart_set_baurate(sm_hal_uart_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param baudrate
 * @return
 */
static inline int32_t sm_hal_uart_set_baurate(sm_hal_uart_t *_this, uint32_t _baudrate)
{
    return _this->proc->set_baudrate (_this, _baudrate);
}
/**
 * @fn int32_t sm_hal_uart_set_databit(sm_hal_uart_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param databit
 * @return
 */
static inline int32_t sm_hal_uart_set_databit(sm_hal_uart_t *_this, uint32_t _databit)
{
    return _this->proc->set_databit (_this, _databit);
}
/**
 * @fn int32_t sm_hal_uart_set_parity(sm_hal_uart_t*, uint32_t)
 * @brief
 *
 * @param _this
 * @param parity
 * @return
 */
static inline int32_t sm_hal_uart_set_parity(sm_hal_uart_t *_this, uint32_t _parity)
{
    return _this->proc->set_parity (_this, _parity);
}
/**
 * @fn int32_t sm_hal_uart_open(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_uart_open(sm_hal_uart_t *_this)
{
    return _this->proc->open (_this);
}
/**
 * @fn int32_t sm_hal_uart_close(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_uart_close(sm_hal_uart_t *_this)
{
    return _this->proc->close (_this);
}
/**
 * @fn int32_t sm_hal_uart_abort_rx(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_uart_abort_rx(sm_hal_uart_t *_this)
{
    return _this->proc->abort_rx (_this);
}
/**
 * @fn int32_t sm_hal_uart_abort_tx(sm_hal_uart_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_uart_abort_tx(sm_hal_uart_t *_this)
{
    return _this->proc->abort_tx (_this);
}
#endif /* hal_INCLUDE_SM_HAL_UART_H_ */
