//
// Created by vuonglk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_CO_H
#define EV_SDK_SM_SV_CO_H

#include <stdint-gcc.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef enum {
    SM_SV_CMD_REBOOT_BP = 0,
    SM_SV_CMD_REBOOT_HMI,
    SM_SV_CMD_REBOOT_PMU,
    SM_SV_CMD_REBOOT_MC
} SM_SV_CO_CMD;

typedef enum {
    SM_SV_CO_ERROR_NONE = 0,
    SM_SV_CO_ERROR_TIMEOUT,
    SM_SV_CO_ERROR_ABORT
}SM_SV_CO_ERROR_CODE;

typedef void sm_sv_co_t;

typedef void (*sm_sv_co_cmd_callback)(SM_SV_CO_CMD cmd, int8_t error_code, void* arg);

sm_sv_co_t* sm_sv_co_create();

int32_t sm_sv_co_push_cmd(sm_sv_co_t* _this, uint32_t _id, SM_SV_CO_CMD _cmd, uint32_t _timeout,
                          sm_sv_co_cmd_callback _callback, void *_arg, bool _force);


int32_t sm_sv_co_process(sm_sv_co_t* _this);

#ifdef __cplusplus
}
#endif

#endif //EV_SDK_SM_SV_CO_H
