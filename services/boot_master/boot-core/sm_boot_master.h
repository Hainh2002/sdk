//
// Created by vnbk on 21/03/2023.
//

#ifndef BOOT_MASTER_SM_BOOT_MASTER_H
#define BOOT_MASTER_SM_BOOT_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_boot_slave.h"

#define SM_BOOT_SLAVE_MAX_SUPPORT   10

typedef void (*sm_boot_master_event_callback_fn_t)(int32_t, int32_t, void*);
typedef struct sm_boot_master sm_boot_master_t;

sm_boot_master_t* sm_boot_master_create(sm_boot_master_event_callback_fn_t _fn, void* _arg);
sm_boot_master_t* sm_boot_master_create_default(sm_boot_master_event_callback_fn_t _fn, void* _arg);
int32_t sm_boot_master_destroy(sm_boot_master_t* _self);

int32_t sm_boot_master_add_slave(sm_boot_master_t* _self,
                                 int32_t _slave_id,
                                 sm_boot_input_if_t* _fw_input,
                                 sm_boot_output_if_t* _fw_output);

int32_t sm_boot_master_remove_slave(sm_boot_master_t* _self, int32_t _slave_id);

int32_t sm_boot_master_process(sm_boot_master_t* _self);

#ifdef __cplusplus
};
#endif

#endif //BOOT_MASTER_SM_BOOT_MASTER_H
