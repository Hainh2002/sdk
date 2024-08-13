/*
 * sm_modem.c
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */


#include "sm_modem.h"
#include "sm_hal_delay.h"
#include <assert.h>
#include <string.h>
#if USING_MODEM
char buff_modem[MODEM_BUFF_SIZE] = {0};
void sm_modem_init(sm_modem_t *modem,sm_hal_io_t *io, sm_hal_uart_t *driver){
    modem->reset_pin = io;
    modem->driver = driver;
    if(modem->buff_length == 0 && modem->buff == NULL){
        modem->buff_length = MODEM_BUFF_SIZE;
//        modem->buff = calloc(modem->buff_length,sizeof(char));
        modem->buff = buff_modem;
    }
    else if(modem->buff_length != 0 && modem->buff == NULL){
//        modem->buff = calloc(modem->buff_length,sizeof(char));
    	modem->buff = buff_modem;
    }
    modem->lock = 0;
}
void sm_modem_deinit(sm_modem_t *modem){
    modem->reset_pin = NULL;
    modem->driver = NULL;
}
int32_t sm_modem_send_cmd(sm_modem_t *modem,char* cmd,char* res_ok, char* res_fail,uint32_t timeout){
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;
    assert( sm_hal_uart_write(modem->driver,(uint8_t*)cmd,strlen(cmd)) == 0);
    memset(modem->buff, 0, modem->buff_length);
    uint32_t time = 0;  
    uint32_t index_buff = 0;
    int32_t res = 0;
    while(1){
		size_t len = sm_hal_uart_read(modem->driver,
				(uint8_t*) modem->buff + index_buff,128);
        if(len > 0){
			index_buff += (uint32_t) len;
			if (strstr(modem->buff, res_ok) != NULL) {
				res = 0;
				break;
			}
			if (strstr(modem->buff, res_fail) != NULL) {
				res = 1;
				break;
			}
        }
        time++;
        if(time > timeout) {
			res = 1;
            break;
        }
        sm_hal_delay_ms(1);
    }
	modem->lock = 0;
    return res;
}
int32_t sm_modem_send_data(sm_modem_t *modem,char* data,size_t len){
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;

	int32_t rc = sm_hal_uart_write(modem->driver, (uint8_t*) data,
			(uint32_t) len);
	modem->lock = 0;
	return rc;
}
void sm_modem_hw_reset(sm_modem_t *modem, uint32_t duration_ms, int logic) {
	if (logic) {
		modem->reset_pin->proc->set_value(modem->reset_pin, 0);
		sm_hal_delay_ms(duration_ms);
		modem->reset_pin->proc->set_value(modem->reset_pin, 1);
	}
	else {
		modem->reset_pin->proc->set_value(modem->reset_pin, 1);
		sm_hal_delay_ms(duration_ms);
		modem->reset_pin->proc->set_value(modem->reset_pin, 0);
	}
}
size_t sm_modem_read(sm_modem_t *modem,char* data,uint32_t len){
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;
	size_t length = sm_hal_uart_read(modem->driver, (uint8_t*) data, len);
	modem->lock = 0;
	return length;
}

size_t sm_modem_peek(sm_modem_t *modem,char* data,uint32_t len){
	while (modem->lock) {
		sm_hal_delay_ms(1);
	}
	modem->lock = 1;
	size_t length = sm_hal_uart_peek(modem->driver, (uint8_t*) data, len);
	modem->lock = 0;
	return length;
}
#endif
