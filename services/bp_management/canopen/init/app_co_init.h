/*
 * app_co_init.h
 *
 *  Created on: Jun 17, 2022
 *      Author: Dakaka
 */

#ifndef CANOPEN_APP_CO_INIT_H_
#define CANOPEN_APP_CO_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CO.h"
#include "od_config.h"

#include "sm_co_if.h"

extern CO CO_DEVICE;
extern CO_OD* p_co_od;

extern void *tpdo_build_data_impl[TPDO_NUMBER];

typedef void (*app_co_received_callback_fn_t)(uint32_t, uint8_t*, void*);

void app_co_init(sm_co_if_t* _co_if, app_co_received_callback_fn_t _fn_callback, void* _arg);

void app_co_free(void);

void app_co_process(void);

void app_co_can_receive(const uint32_t can_id, uint8_t *data);

void app_co_sync(bool _enable);

sm_co_if_t* app_co_get_if();

void app_co_set_if(sm_co_if_t* _co_if);

#ifdef __cplusplus
};
#endif

#endif /* CANOPEN_APP_CO_INIT_H_ */

