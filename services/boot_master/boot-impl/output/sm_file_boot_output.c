//
// Created by vnbk on 28/03/2023.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sm_boot_impl.h"
#include "sm_logger.h"

#define _imp(p) ((sm_boot_output_impl_t*)(p))

#define FILE_NAME_PREFIX "./selex_fw_slave.bin"

#define TAG "sm_file_boot_output"

typedef struct {
    sm_boot_slave_event_callback_fn_t m_callback;
    void* m_arg;
}sm_boot_output_event_t;

typedef struct{
    sm_boot_output_if_t m_base;

    sm_boot_output_event_t m_events[SM_BOOT_SLAVE_EVENT_NUMBER];
    
    sm_fw_signature_t m_fw_signature;
    sm_segment_t* m_segment;
    int32_t m_total_seg;

    int32_t m_count;

    char m_file_name[50];
}sm_boot_output_impl_t;

static char *rand_string(char *str, size_t size)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }
    return str;
}

int32_t sm_desktop_file_init(sm_boot_output_if_t* _self){
    strcpy(_imp(_self)->m_file_name, FILE_NAME_PREFIX);
//    rand_string(_imp(_self)->m_file_name + strlen(FILE_NAME_PREFIX), 5);

    FILE* fw_file = fopen(_imp(_self)->m_file_name, "w");
    fclose(fw_file);
    return 0;
}

int32_t sm_desktop_file_free(sm_boot_output_if_t* _self){
    (void)_self;
    return 0;
}

int32_t sm_desktop_file_reg_event_callback(sm_boot_output_if_t* _self, SM_BOOT_SLAVE_EVENT _event, sm_boot_slave_event_callback_fn_t _fn, void* _arg){
    if(!_fn){
        return -1;
    }
    _imp(_self)->m_events[_event].m_callback = _fn;
    _imp(_self)->m_events[_event].m_arg = _arg;
    return 0;
}

int32_t sm_desktop_file_request_upgrade(sm_boot_output_if_t* _self, int32_t _src_id){
    (void)_src_id;
    uint8_t ready = 1;
    _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&ready, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);
    return 0;
}

int32_t sm_desktop_file_set_fw_info(sm_boot_output_if_t* _self, const sm_fw_signature_t* _fw_signature){
    sm_fw_signature_clone(&_imp(_self)->m_fw_signature, _fw_signature);
    uint8_t ready = 1;
    _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&ready, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
    return 0;
}

int32_t sm_desktop_file_set_seg_fw(sm_boot_output_if_t* _self, const sm_segment_t* _seg){
    FILE* fw_file = fopen(_imp(_self)->m_file_name, "a");
    if(fw_file < 0){
        return -1;
    }
    size_t len = fwrite(_seg->m_data, sizeof(char), _seg->m_length, fw_file);

    fclose(fw_file);

    uint8_t success = 1;
    _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success, _imp(_self)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);

    return (int32_t)len;
}

static const sm_boot_output_if_proc_t g_boot_output_proc_default = {
        .init = sm_desktop_file_init,
        .free = sm_desktop_file_free,
        .reg_event_callback = sm_desktop_file_reg_event_callback,
        .request_upgrade = sm_desktop_file_request_upgrade,
        .set_fw_info = sm_desktop_file_set_fw_info,
        .set_seg_fw = sm_desktop_file_set_seg_fw,
        .process = NULL
};

static sm_boot_output_impl_t g_boot_output_default = {
        .m_base.m_proc = &g_boot_output_proc_default,
};

sm_boot_output_if_t* sm_get_file_boot_output(){
    return &g_boot_output_default.m_base;
}