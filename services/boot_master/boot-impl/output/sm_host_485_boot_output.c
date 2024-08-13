//
// Created by vnbk on 03/11/2023.
//
#include <stdio.h>
#include <stdbool.h>

#include "sm_boot_impl.h"
#include "sm_elapsed_timer.h"
#include "sm_time_utils.h"
#include "sm_host.h"

#define TAG "sm_host_485_boot_output"

#define _impl(p) ((sm_boot_host_sync_impl_t*)(p))

#define HOST_RS485_PACKET_SEG_LEN     128
#define HOST_RS485_PACKET_MAX_LEN     (HOST_RS485_PACKET_SEG_LEN + 10)

#define STATE_REQUEST_UPGRADE_TIMEOUT   200 //ms
#define STATE_SEND_SEG_INFO_TIMEOUT     200 //ms
#define STATE_SEND_SEG_DATA_TIMEOUT     2000 //ms
#define STATE_SEND_FW_INFO_TIMEOUT      200 //ms
#define STATE_SEND_SYNC_TIMEOUT         2000 //ms

typedef enum {
    UPGRADE_STATE_IDLE,
    UPGRADE_STATE_REQUEST_UPGRADE,
    UPGRADE_STATE_SEND_SEG_INFO,
    UPGRADE_STATE_SEND_SEG_DATA,
    UPGRADE_STATE_SEND_FW_INFO,
    UPGRADE_STATE_NUMBER
}HOST_RS485_UPGRADE_PROCESS_STATE;

typedef struct {
    sm_boot_slave_event_callback_fn_t m_callback;
    void* m_arg;
}sm_boot_output_event_t;

typedef struct {
    sm_boot_output_if_t m_base;
    int32_t m_src_id;

    sm_boot_output_event_t m_events[SM_BOOT_SLAVE_EVENT_NUMBER];

    const sm_fw_signature_t* m_fw_signature;
    const sm_segment_t* m_segment;

    uint8_t m_state;
    int32_t m_seg_offset;
    bool    m_last_packet;
    elapsed_timer_t m_timeout;
    int32_t m_retry;

    sm_host_t* m_host;
    void* m_host_arg;
}sm_boot_host_sync_impl_t;

static int32_t sm_host_cmd_callback(int32_t _cmd, const uint8_t* _data, int32_t _len, void* _arg);

static int32_t sm_host_rs485_build_request_update_packet(sm_boot_output_if_t* _this, uint8_t* _buf, int32_t _max_len){
    uint8_t* buf = _buf;
    int32_t len = 0;

    buf[len++] = SM_SUB_CMD_REQUEST;
    return len;
}

static int32_t sm_host_rs485_build_segment_data_packet(sm_boot_output_if_t* _this, uint8_t* _data, int32_t _len, int32_t _offset){
    uint8_t* packet = _data;
    int32_t len = 0;

    packet[len++] = SM_SUB_CMD_SEND_SEGMENT_DATA;
    memcpy(&packet[len], &_impl(_this)->m_segment->m_data[_offset], _len);

    len += _len;
    return len;
}

static int32_t sm_host_rs485_build_segment_info_packet(sm_boot_output_if_t* _this, uint8_t* _buf, int32_t _max_len){
    uint8_t* packet = _buf;
    int32_t len = 0;

    packet[len++] = SM_SUB_CMD_SEND_SEGMENT_INFO;

    /// Segment Index;
    packet[len++] = 0;

    /// Last segment
    if(_impl(_this)->m_segment->m_is_last){
        packet[len++] = 1;
    }else{
        packet[len++] = 0;
    }

    /// Address segment
    packet[len++] = (_impl(_this)->m_segment->m_addr >> 24) & 0xFF;
    packet[len++] = (_impl(_this)->m_segment->m_addr >> 16) & 0xFF;
    packet[len++] = (_impl(_this)->m_segment->m_addr >> 8) & 0xFF;
    packet[len++] = (_impl(_this)->m_segment->m_addr) & 0xFF; // Address Segment

    /// Size segment
    packet[len++] = (_impl(_this)->m_segment->m_size >> 24) & 0xFF;
    packet[len++] = (_impl(_this)->m_segment->m_size >> 16) & 0xFF;
    packet[len++] = (_impl(_this)->m_segment->m_size >> 8) & 0xFF;
    packet[len++] = (_impl(_this)->m_segment->m_size) & 0xFF; // Segment Size

    /// CRC Segment
    packet[len++] = (_impl(_this)->m_segment->m_crc >> 8) & 0xFF;
    packet[len++] = (_impl(_this)->m_segment->m_crc) & 0xFF;

    return len;
}

static int32_t sm_host_rs485_build_fw_info_packet(sm_boot_output_if_t* _this, uint8_t* _buf, int32_t _max_len){
    uint8_t* packet = _buf;
    int32_t len = 0;
    int32_t index = 0;

    packet[len++] = SM_SUB_CMD_FW_INFO;

    /// Version
    for(index = 0; index < SM_FW_VERSION_LENGTH; index++){
        packet[len++] = _impl(_this)->m_fw_signature->m_version[index];
    }

    /// Size of firmware
    packet[len++] = (_impl(_this)->m_fw_signature->m_size >> 24) & 0xFF;
    packet[len++] = (_impl(_this)->m_fw_signature->m_size >> 16) & 0xFF;
    packet[len++] = (_impl(_this)->m_fw_signature->m_size >> 8) & 0xFF;
    packet[len++] = (_impl(_this)->m_fw_signature->m_size) & 0xFF;

    /// Start Address of firmware
    packet[len++] = (_impl(_this)->m_fw_signature->m_addr >> 24) & 0xFF;
    packet[len++] = (_impl(_this)->m_fw_signature->m_addr >> 16) & 0xFF;
    packet[len++] = (_impl(_this)->m_fw_signature->m_addr >> 8) & 0xFF;
    packet[len++] = (_impl(_this)->m_fw_signature->m_addr) & 0xFF;

    /// CRC
    packet[len++] = (_impl(_this)->m_fw_signature->m_crc >> 8) & 0xFF;
    packet[len++] = (_impl(_this)->m_fw_signature->m_crc) & 0xFF;

    return len;
}

static int32_t sm_get_event_by_state(sm_boot_output_if_t* _this){
    switch (_impl(_this)->m_state) {
        case UPGRADE_STATE_IDLE:
            break;
        case UPGRADE_STATE_REQUEST_UPGRADE:
            return SM_BOOT_SLAVE_CONFIRM_READY;
        case UPGRADE_STATE_SEND_SEG_INFO:
        case UPGRADE_STATE_SEND_SEG_DATA:
            return SM_BOOT_SLAVE_CONFIRM_SEG;
        case UPGRADE_STATE_SEND_FW_INFO:
            return SM_BOOT_SLAVE_CONFIRM_FW_INFO;
    }
    return -1;
}

static void sm_boot_host_sync_notify(sm_boot_output_if_t* _this, bool _isSuccess){
    bool success = _isSuccess;
    int32_t event = sm_get_event_by_state(_this);
    if(event < 0){
        printf( "NOT Match any events in this state");
        return;
    }
    if(_impl(_this)->m_events[event].m_callback) {
        _impl(_this)->m_events[event].m_callback(&success,
                                                 _impl(_this)->m_events[event].m_arg);
    }
}

int32_t sm_boot_host_sync_process(sm_boot_output_if_t* _this){  //hostsend
    int len = -1;
    uint8_t packet[HOST_RS485_PACKET_MAX_LEN] = {0,};

    if(!_this){
        return -1;
    }

    switch (_impl(_this)->m_state) {
        case UPGRADE_STATE_IDLE:
            break;
        case UPGRADE_STATE_REQUEST_UPGRADE:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_timeout)) {
                len = sm_host_rs485_build_request_update_packet(_this, packet, 1);

                sm_host_send_cmd_sync(_impl(_this)->m_host, SM_CMD_UPGRADE_FW, packet, len, STATE_REQUEST_UPGRADE_TIMEOUT, sm_host_cmd_callback, _impl(_this));

                elapsed_timer_resetz(&_impl(_this)->m_timeout,  STATE_REQUEST_UPGRADE_TIMEOUT);
            }
            break;
        case UPGRADE_STATE_SEND_SEG_INFO:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_timeout)){
                len = sm_host_rs485_build_segment_info_packet(_this, packet, HOST_RS485_PACKET_MAX_LEN);
                if(!sm_host_send_cmd_sync(_impl(_this)->m_host, SM_CMD_UPGRADE_FW, packet, len, STATE_SEND_SYNC_TIMEOUT, sm_host_cmd_callback, _impl(_this))){
                    _impl(_this)->m_retry++;
                    if(_impl(_this)->m_retry >= 5){
                        sm_boot_host_sync_notify(_this, false);
                    }
                }
                elapsed_timer_reset(&_impl(_this)->m_timeout);
            }

            break;
        case UPGRADE_STATE_SEND_SEG_DATA:
            if(_impl(_this)->m_seg_offset + HOST_RS485_PACKET_SEG_LEN >= _impl(_this)->m_segment->m_length){
                len = _impl(_this)->m_segment->m_length - _impl(_this)->m_seg_offset;
                _impl(_this)->m_last_packet = true;
            }else{
                len = HOST_RS485_PACKET_SEG_LEN;
            }
            if(len <= 0){
                return 0;
            }
            len = sm_host_rs485_build_segment_data_packet(_this, packet, len, _impl(_this)->m_seg_offset);

            if(!sm_host_send_cmd_sync(_impl(_this)->m_host, SM_CMD_UPGRADE_FW, packet, len, STATE_SEND_SEG_DATA_TIMEOUT, sm_host_cmd_callback, _impl(_this))){
                sm_boot_host_sync_notify(_this, false);
            }


            _impl(_this)->m_seg_offset += HOST_RS485_PACKET_SEG_LEN;
            elapsed_timer_resetz(&_impl(_this)->m_timeout,  STATE_SEND_SEG_DATA_TIMEOUT);
            break;
        case UPGRADE_STATE_SEND_FW_INFO:
            len = sm_host_rs485_build_fw_info_packet(_this, packet, HOST_RS485_PACKET_MAX_LEN);

            if(!sm_host_send_cmd_sync(_impl(_this)->m_host, SM_CMD_UPGRADE_FW, packet, len, STATE_SEND_FW_INFO_TIMEOUT, sm_host_cmd_callback, _impl(_this))){
                sm_boot_host_sync_notify(_this, false);
            }

            elapsed_timer_resetz(&_impl(_this)->m_timeout,  STATE_SEND_FW_INFO_TIMEOUT);

            break;
        default:
            _impl(_this)->m_state = UPGRADE_STATE_IDLE;
    }

    return 0;
}

static int32_t sm_host_cmd_callback(int32_t _cmd, const uint8_t* _data, int32_t _len, void* _arg){ //callbackhost
    sm_boot_output_if_t* _this = (sm_boot_output_if_t*)_arg;
    if(_len < 2 || !_this){
        return -1;
    }

    if(_cmd == SM_CMD_UPGRADE_FW){
        int8_t ret = (int8_t)_data[0];
        uint8_t sub_cmd = _data[1];
        if(ret < 0){
            printf("Upgrade firmware command is FAILURE: sub_cmd: 0x%02X\n", sub_cmd);
            if(sub_cmd == SM_SUB_CMD_REQUEST && _impl(_this)->m_state == UPGRADE_STATE_REQUEST_UPGRADE){
                printf("Device is already in upgrading mode\n");
                delayMs(500);
                sm_boot_host_sync_notify(_this, true);
                return 0;
            }
            sm_boot_host_sync_notify(_this, false);
        }else{
            if(sub_cmd == SM_SUB_CMD_REQUEST && _impl(_this)->m_state == UPGRADE_STATE_REQUEST_UPGRADE){
                delayMs(500);
                sm_boot_host_sync_notify(_this, true);
                return 0;
            }

            if(sub_cmd == SM_SUB_CMD_SEND_SEGMENT_INFO && _impl(_this)->m_state == UPGRADE_STATE_SEND_SEG_INFO){
                _impl(_this)->m_state = UPGRADE_STATE_SEND_SEG_DATA;
                return 0;
            }

            if(sub_cmd == SM_SUB_CMD_SEND_SEGMENT_DATA && _impl(_this)->m_state == UPGRADE_STATE_SEND_SEG_DATA){
                if(_impl(_this)->m_last_packet){
                    sm_boot_host_sync_notify(_this, true);
                }
                return 0;
            }

            if(sub_cmd == SM_SUB_CMD_FW_INFO && _impl(_this)->m_state == UPGRADE_STATE_SEND_FW_INFO){
                sm_boot_host_sync_notify(_this, true);
                return 0;
            }
        }
        return 0;
    }
    return -1;
}

int32_t sm_boot_host_sync_init(sm_boot_output_if_t* _this);
int32_t sm_boot_host_sync_free(sm_boot_output_if_t* _this);
int32_t sm_boot_host_sync_reg_event_callback(sm_boot_output_if_t* _this,
                                              SM_BOOT_SLAVE_EVENT _event,
                                              sm_boot_slave_event_callback_fn_t _fn,
                                              void* _arg);
int32_t sm_boot_host_sync_request_upgrade(sm_boot_output_if_t* _this, int32_t _src_id);
int32_t sm_boot_host_sync_set_fw_info(sm_boot_output_if_t* _this, const sm_fw_signature_t* _fw_info);
int32_t sm_boot_host_sync_set_seg_fw(sm_boot_output_if_t* _this, const sm_segment_t* _seg);

static const sm_boot_output_if_proc_t g_boot_output_proc_default = { //m_cb
        .init = sm_boot_host_sync_init,
        .free = sm_boot_host_sync_free,
        .reg_event_callback = sm_boot_host_sync_reg_event_callback,
        .request_upgrade = sm_boot_host_sync_request_upgrade,
        .set_fw_info = sm_boot_host_sync_set_fw_info,
        .set_seg_fw = sm_boot_host_sync_set_seg_fw,
        .process = sm_boot_host_sync_process
};

static sm_boot_host_sync_impl_t g_boot_host_rs485_default = {
        .m_base.m_proc = &g_boot_output_proc_default,
        .m_src_id = -1,
        .m_segment = NULL,
        .m_fw_signature = NULL,
        .m_retry = 0,
        .m_events = {NULL, NULL, NULL, NULL, NULL}
};

/***
 *
 * @param _this
 * @return
 */
int32_t sm_boot_host_sync_init(sm_boot_output_if_t* _this){
    if(!_this){
        printf( "Boot Host RS485 is NOT created");
        return -1;
    }

    _impl(_this)->m_src_id = -1;
    _impl(_this)->m_fw_signature = NULL;

    _impl(_this)->m_segment = NULL;
    _impl(_this)->m_last_packet = false;
    _impl(_this)->m_seg_offset = 0;
    _impl(_this)->m_state = UPGRADE_STATE_IDLE;
    _impl(_this)->m_retry = 0;

    return 0;
}

int32_t sm_boot_host_sync_free(sm_boot_output_if_t* _this){
    _impl(_this)->m_src_id = -1;
    _impl(_this)->m_fw_signature = NULL;

    _impl(_this)->m_segment = NULL;
    _impl(_this)->m_last_packet = false;
    _impl(_this)->m_seg_offset = 0;
    _impl(_this)->m_state = UPGRADE_STATE_IDLE;

//    _impl(_this)->m_host = NULL;
    return 0;
}

int32_t sm_boot_host_sync_reg_event_callback(sm_boot_output_if_t* _this,
                                              SM_BOOT_SLAVE_EVENT _event,
                                              sm_boot_slave_event_callback_fn_t _fn,
                                              void* _arg){
    if(!_this || !_fn){
        return -1;
    }
    _impl(_this)->m_events[_event].m_callback = _fn;
    _impl(_this)->m_events[_event].m_arg = _arg;
    return 0;
}

int32_t sm_boot_host_sync_request_upgrade(sm_boot_output_if_t* _this, int32_t _src_id){
    if(!_this){
        return -1;
    }

    if(_impl(_this)->m_src_id >= 0 && _impl(_this)->m_src_id != _src_id){
        printf("Host RS485 is upgrading progress with other Device that Node Id = %d\n", _impl(_this)->m_src_id);
        return -1;
    }

    _impl(_this)->m_src_id = _src_id;
    _impl(_this)->m_state = UPGRADE_STATE_REQUEST_UPGRADE;
    _impl(_this)->m_seg_offset = 0;
    _impl(_this)->m_last_packet = false;

    sm_host_set_addr(_impl(_this)->m_host, _impl(_this)->m_src_id + 1);

    printf("Request upgrade firmware, Source ID: %d\n", _src_id);

    return 0;
}

int32_t sm_boot_host_sync_set_fw_info(sm_boot_output_if_t* _this, const sm_fw_signature_t* _fw_info){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_src_id < 0){
        printf("host_rs485 is NOT active or busy");
        return -1;
    }

    _impl(_this)->m_fw_signature = _fw_info;
    printf( "New fw with version: %X.%X.%X\n size: %d \n crc: %d\n",
            _fw_info->m_version[0],
            _fw_info->m_version[1],
            _fw_info->m_version[2],
            _fw_info->m_size,
            _fw_info->m_crc);

    _impl(_this)->m_state = UPGRADE_STATE_SEND_FW_INFO;
    _impl(_this)->m_seg_offset = 0;
    _impl(_this)->m_last_packet = false;

    return 0;
}

int32_t sm_boot_host_sync_set_seg_fw(sm_boot_output_if_t* _this, const sm_segment_t* _seg){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_src_id < 0){
        printf( "host_rs485 is NOT active or busy");
        return -1;
    }

    _impl(_this)->m_segment = _seg;
    _impl(_this)->m_last_packet = false;
    _impl(_this)->m_seg_offset = 0;
    _impl(_this)->m_state = UPGRADE_STATE_SEND_SEG_INFO;
    _impl(_this)->m_retry = 0;

    elapsed_timer_resetz(&_impl(_this)->m_timeout, STATE_SEND_SEG_INFO_TIMEOUT);

    printf("Address Segment: 0x%2X\n", _seg->m_addr);

    return 0;
}

sm_boot_output_if_t* sm_get_host_sync_boot_output(sm_host_t* _host,
                                                   void* _arg){
    if(!_host){
        return NULL;
    }

    g_boot_host_rs485_default.m_base.m_proc = &g_boot_output_proc_default;
    g_boot_host_rs485_default.m_src_id = -1;
    g_boot_host_rs485_default.m_segment = NULL;
    g_boot_host_rs485_default.m_fw_signature = NULL;
    g_boot_host_rs485_default.m_seg_offset = 0;
    g_boot_host_rs485_default.m_last_packet = false;

    g_boot_host_rs485_default.m_host = _host;
    g_boot_host_rs485_default.m_host_arg = _arg;

    elapsed_timer_resetz(&g_boot_host_rs485_default.m_timeout, STATE_REQUEST_UPGRADE_TIMEOUT);

    for(int index = 0; index < SM_BOOT_SLAVE_EVENT_NUMBER; index++){
        g_boot_host_rs485_default.m_events[index].m_callback = NULL;
        g_boot_host_rs485_default.m_events[index].m_arg = NULL;
    }

    return &g_boot_host_rs485_default.m_base;
}