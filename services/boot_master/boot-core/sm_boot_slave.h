//
// Created by vnbk on 28/03/2023.
//

#ifndef BOOT_MASTER_SM_BOOT_SLAVE_H
#define BOOT_MASTER_SM_BOOT_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sm_fw.h"


typedef struct sm_boot_if{
    int32_t (*init)();
    int32_t (*free)();
    int32_t (*get_fw_info)(sm_fw_signature_t*);
    int32_t (*get_total_seg)();
    int32_t (*get_seg_fw)(sm_segment_t*);
    int32_t (*seg_upgrade_completed)(int32_t);
    int32_t (*process)();
}sm_boot_input_if_t;

typedef enum {
    SM_BOOT_SLAVE_CONFIRM_READY,
    SM_BOOT_SLAVE_CONFIRM_FW_INFO,
    SM_BOOT_SLAVE_CONFIRM_SEG,
    SM_BOOT_SLAVE_CONFIRM_FINISH_DOWNLOAD,
    SM_BOOT_SLAVE_CONFIRM_ABORT,
    SM_BOOT_SLAVE_EVENT_NUMBER
}SM_BOOT_SLAVE_EVENT;

typedef int32_t (*sm_boot_slave_event_callback_fn_t)(void*, void*);
typedef struct sm_boot_output_if sm_boot_output_if_t;
typedef struct sm_boot_output_if_proc{
    int32_t (*init)(sm_boot_output_if_t*);
    int32_t (*free)(sm_boot_output_if_t*);
    int32_t (*reg_event_callback)(sm_boot_output_if_t*, SM_BOOT_SLAVE_EVENT, sm_boot_slave_event_callback_fn_t, void*);
    int32_t (*request_upgrade)(sm_boot_output_if_t*, int32_t);
    int32_t (*set_fw_info)(sm_boot_output_if_t*, const sm_fw_signature_t*);
    int32_t (*set_seg_fw)(sm_boot_output_if_t*, const sm_segment_t*);
    int32_t (*process)(sm_boot_output_if_t*);
}sm_boot_output_if_proc_t;

struct sm_boot_output_if {
    const sm_boot_output_if_proc_t* m_proc;
};

typedef struct sm_boot_slave sm_boot_slave_t;

sm_boot_slave_t* sm_boot_slave_create(int32_t id, sm_boot_input_if_t* _input, sm_boot_output_if_t* _output, void* _master);
sm_boot_slave_t* sm_boot_slave_create_default(int32_t id, sm_boot_input_if_t* _input, sm_boot_output_if_t* _output, void* _master);
int32_t sm_boot_slave_destroy(sm_boot_slave_t* _self);

void* sm_boot_slave_get_master(sm_boot_slave_t* _self);
int32_t sm_boot_slave_get_id(sm_boot_slave_t*);
int32_t sm_boot_slave_update_if(sm_boot_slave_t*,  sm_boot_input_if_t* _input, sm_boot_output_if_t* _output);
int32_t sm_boot_slave_reg_event(sm_boot_slave_t* _self, SM_BOOT_SLAVE_EVENT _event, sm_boot_slave_event_callback_fn_t _fn_callback, void* _arg);

int32_t sm_boot_slave_process(sm_boot_slave_t* _self);

#ifdef __cplusplus
}
#endif

#endif //BOOT_MASTER_SM_BOOT_SLAVE_H
