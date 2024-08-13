//
// Created by vnbk on 12/09/2023.
//

#ifndef BSS_SDK_SM_OTA_BOOT_IMPL_H
#define BSS_SDK_SM_OTA_BOOT_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sm_boot_slave.h"
#include "sm_mb_slave_impl.h"

typedef int32_t (*host_send_fn_t)(const uint8_t*,int32_t, void*);
typedef int32_t (*host_recv_fn_t)(uint8_t*, int32_t, void*);

sm_boot_output_if_t* sm_get_co_boot_output();
sm_boot_output_if_t* sm_get_host_rs485_boot_output(host_send_fn_t _send_fn,
                                                   host_recv_fn_t _recv_fn,
                                                   void* _arg);
sm_boot_output_if_t* sm_get_file_boot_output();

sm_boot_input_if_t* sm_get_file_boot_input(const char* file_path);

sm_boot_input_if_t* sm_get_modbus_rtu_boot_input(uint8_t _address,
                                                 const sm_fw_signature_t* _fw_info,
                                                 sm_mb_slave_t* _mb,
                                                 void* _arg);

#ifdef __cplusplus
};
#endif

#endif //BSS_SDK_SM_OTA_BOOT_IMPL_H
