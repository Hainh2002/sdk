//
// Created by vuonglk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_BOOT_MASTER_H
#define EV_SDK_SM_SV_BOOT_MASTER_H

#ifdef __cplusplus
extern "C"{
#endif

#include "stdint.h"
#include "stdbool.h"
#include "sm_elapsed_timer.h"
#include "sm_vector.h"

#include "sm_boot_master.h"

typedef void sm_sv_boot_master_t;

typedef enum
{
    SM_SV_BOOT_MASTER_INPUT_FILE = 0,
    SM_SV_BOOT_MASTER_INPUT_MODBUS_RTU,
    SM_SV_BOOT_MASTER_INPUT_TYPE_NUMBER
} SM_SV_BOOT_MASTER_INPUT_TYPE;

typedef enum
{
    SM_SV_BOOT_MASTER_OUTPUT_CANOPEN = 0,
    SM_SV_BOOT_MASTER_OUTPUT_FILE,
    SM_SV_BOOT_MASTER_OUTPUT_HOST_SYNC,
    SM_SV_BOOT_MASTER_OUTPUT_MODBUS_RTU,
    SM_SV_BOOT_MASTER_OUTPUT_TYPE_NUMBER
} SM_SV_BOOT_MASTER_OUTPUT_TYPE;

typedef enum
{
    SM_SV_BOOT_MASTER_ERR_NONE = 0,
    SM_SV_BOOT_MASTER_ERR_INTERNAL = -1,
    SM_SV_BOOT_MASTER_ERR_TIMEOUT = -2,
    SM_SV_BOOT_MASTER_ERR_REBOOT = -3,
    SM_SV_BOOT_MASTER_ERR_DOWNLOAD = -4,
    SM_SV_BOOT_MASTER_ERR_NOT_SUPPORT = -5,
    SM_SV_BOOT_MASTER_ERR_UNKNOWN = -6
} SM_SV_BOOT_MASTER_UPGRADING_ERROR;


typedef int32_t (*sm_boot_master_send_if_fn)(const uint8_t* _data, int32_t _len, int32_t _timeout, void* _arg);
typedef int32_t (*sm_boot_master_receive_if_fn)(uint8_t* _buf, int32_t _len, int32_t _timeout, void* _arg);
typedef struct sm_sv_boot_master_if_fn
{
    sm_boot_master_send_if_fn send_if;
    sm_boot_master_receive_if_fn receive_if;
    void* arg;
} sm_sv_boot_master_if_fn_t;


typedef void (*sm_boot_master_callback_reboot)(uint8_t _devId, uint8_t _issSuccess, void* _arg);
typedef void (*sm_boot_master_callback_on_upgrading_status)(uint8_t _devId, const char* _ver, uint8_t _err, void* _arg);
typedef void (*sm_boot_master_callback_finish_upgrading)(void* _arg);
typedef struct sm_sv_boot_master_events_callback
{
    sm_boot_master_callback_reboot reboot_dev;
    sm_boot_master_callback_on_upgrading_status on_upgrading_status;
    sm_boot_master_callback_finish_upgrading finish_upgrading;
    void* arg;
} sm_sv_boot_master_events_callback_t;


//If device not need to reboot, just leave reboot_dev_if is NULL
//If device not need to wait for reboot, just leave check_reboot_status_if is NULL

typedef int (*sm_boot_master_reboot_dev_if)(uint8_t _id, uint32_t _timeout, void* _arg);
typedef bool (*sm_boot_master_check_dev_reboot_status_if)(uint8_t _id, void* _arg);
typedef bool (*sm_boot_master_check_dev_upgrading_condition_if)(uint8_t _id, void* _arg);
typedef struct sm_sv_boot_dev_if_fn
{
    sm_boot_master_reboot_dev_if reboot_dev;
    sm_boot_master_check_dev_reboot_status_if get_dev_reboot_status;
    sm_boot_master_check_dev_upgrading_condition_if check_dev_upgrading_condition;
    void* arg;
} sm_sv_boot_dev_if_fn_t;

typedef struct sm_sv_boot_dev_info
{
    uint8_t m_devId;
    const char* m_ver;
    const char* m_path;
    sm_sv_boot_dev_if_fn_t* m_if;
    elapsed_timer_t m_timeout;
    uint8_t m_state;
    void* m_arg;
} sm_sv_boot_dev_info_t;


sm_sv_boot_master_t* sm_sv_boot_master_create(sm_sv_boot_master_events_callback_t* _eventCallback);

int sm_sv_boot_master_set_output_if_fn(sm_sv_boot_master_t* _this, uint8_t _inputType, sm_boot_master_send_if_fn _sendIf,
                                sm_boot_master_receive_if_fn _recvIf, void* _arg);

int sm_sv_boot_master_set_input_if_fn(sm_sv_boot_master_t* _this, uint8_t _outputType, sm_boot_master_send_if_fn _sendIf,
                                sm_boot_master_receive_if_fn _recvIf, void* _arg);

int sm_sv_boot_master_request_upgrade(sm_sv_boot_master_t* _this, uint8_t _devId,
                                      const char* _ver, const char* _fwPath, sm_sv_boot_dev_if_fn_t* _if, void* _arg);

void sm_sv_boot_master_process(sm_sv_boot_master_t* _this);

int sm_sv_boot_master_destroy(sm_sv_boot_master_t* _this);

#ifdef __cplusplus
}
#endif

#endif //EV_SDK_SM_SV_BOOT_MASTER_H
