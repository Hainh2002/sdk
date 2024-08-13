/*
 * sm_modem.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef COMMON_MODEM_SM_MODEM_H_
#define COMMON_MODEM_SM_MODEM_H_

#include "sm_hal_uart.h"
#include "sm_hal_io.h"
#include "sm_config.h"

#if USING_MODEM

#define MODEM_BUFF_SIZE 2048

typedef struct sm_modem sm_modem_t;

struct sm_modem
{
    /* data */
    sm_hal_io_t *reset_pin;
    sm_hal_uart_t *driver;
    char* buff;
    uint32_t buff_length;
	volatile bool lock;
};

/**
 * @brief
 * @param modem
 * @param io
 * @param driver
 */
void sm_modem_init(sm_modem_t *modem, sm_hal_io_t *io, sm_hal_uart_t *driver);

/**
 * @brief
 * @param modem
 */
void sm_modem_deinit(sm_modem_t *modem);

/**
 * @brief
 * @param modem
 * @param cmd
 * @param res_ok
 * @param res_fail
 * @param timeout
 * @return
 */
int32_t sm_modem_send_cmd(sm_modem_t *modem,char* cmd,char* res_ok, char* res_fail,uint32_t timeout);

/**
 * @brief
 * @param modem
 * @param duration_ms
 * @param logic
 */
void sm_modem_hw_reset(sm_modem_t *modem, uint32_t duration_ms, int logic);

/**
 * @brief
 * @param modem
 * @param data
 * @param len
 * @return
 */
int32_t sm_modem_send_data(sm_modem_t *modem,char* data,size_t len);

/**
 * @brief
 * @param modem
 * @param data
 * @param len
 * @return
 */
size_t sm_modem_read(sm_modem_t *modem,char* data,uint32_t len);
size_t sm_modem_peek(sm_modem_t *modem,char* data,uint32_t len);
#endif
#endif /* COMMON_MODEM_SM_MODEM_H_ */
