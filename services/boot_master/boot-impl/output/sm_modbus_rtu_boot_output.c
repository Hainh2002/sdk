//
// Created by vnbk on 03/11/2023.
//
#include <stdio.h>
#include <stdbool.h>

#ifdef __linux
#include <unistd.h>
#include <malloc.h>

#endif

#include "sm_boot_impl.h"
#include "sm_elapsed_timer.h"
#include "sm_time_utils.h"
#include "sm_modbus_define.h"

#define TAG "sm_modbus_rtu_boot_output"

#define _impl(p) ((sm_boot_modbus_rtu_impl_t*)(p))

#define STATE_REQUEST_UPGRADE_TIMEOUT   500 //ms
#define STATE_SEND_SEG_INFO_TIMEOUT     1000 //ms
#define STATE_SEND_SEG_DATA_TIMEOUT     1000 //ms
#define STATE_SEND_FW_INFO_TIMEOUT      1000 //ms

#define NUMBER_OF_RETRY     10

#define FILE_SEG_FRAME_SIZE             (BSS_MODBUS_FILE_MAX_LEN*2)

typedef enum STATE_OTA{
    OTA_STATUS_IDLE,
    OTA_STATUS_BUSY,
    OTA_STATUS_FINISHED,
    OTA_STATUS_FAILURE,
    OTA_STATUS_UNKNOWN
}STATUS_OTA;


typedef enum {
    UPGRADE_STATE_IDLE,
    UPGRADE_STATE_REQUEST_UPGRADE,
    UPGRADE_STATE_SEND_SEG_INFO,
    UPGRADE_STATE_SEND_SEG_DATA,
    UPGRADE_STATE_SEND_FW_INFO,
    UPGRADE_STATE_NUMBER
}modbus_rtu_UPGRADE_PROCESS_STATE;

typedef struct {
    sm_boot_slave_event_callback_fn_t m_callback;
    void* m_arg;
}sm_boot_output_event_t;

typedef struct {
    sm_boot_output_if_t m_base;
    int32_t m_src_id;

    sm_boot_output_event_t m_events[SM_BOOT_SLAVE_EVENT_NUMBER];

    sm_fw_signature_t m_fw_signature;
    const sm_segment_t* m_segment;

    uint8_t m_state;
    int32_t m_total_frame;
    int32_t m_frame_index;

    elapsed_timer_t m_timeout;
    int32_t m_retry;

    void* m_modbus_master;
    modbus_rtu_write_multiple_registers_fn_t m_modbus_write_mul_regs_fn;
    modbus_rtu_read_holding_registers_fn_t m_modbus_read_holding_regs_fn;

}sm_boot_modbus_rtu_impl_t;


static int32_t sm_modbus_rtu_build_segment_data_packet(sm_boot_output_if_t* _this, uint16_t* _data){
    uint16_t* packet = _data;
    int32_t len = 0;

    int32_t buf_len = (_impl(_this)->m_frame_index >= (_impl(_this)->m_total_frame - 1)) ?
                      (_impl(_this)->m_segment->m_size - (_impl(_this)->m_frame_index*FILE_SEG_FRAME_SIZE)) : FILE_SEG_FRAME_SIZE;

    packet[len++] = _impl(_this)->m_frame_index;

    memcpy((uint8_t*)&packet[len],
           &_impl(_this)->m_segment->m_data[_impl(_this)->m_frame_index*FILE_SEG_FRAME_SIZE], buf_len);

    return (len + buf_len/2);
}

static int32_t sm_modbus_rtu_build_segment_info_packet(sm_boot_output_if_t* _this, uint16_t* _buf){
    uint16_t* packet = _buf;
    int32_t len = 0;

    if(!_impl(_this)->m_segment){
        return -1;
    }

    packet[len] = BSS_MODBUS_FILE_TYPE_RECORD;
    packet[len] = (packet[len] << 8) | (_impl(_this)->m_segment->m_size & 0xFF);
    len++;

    packet[len++] = _impl(_this)->m_segment->m_size >> 8 ;
    packet[len++] = _impl(_this)->m_segment->m_crc;
    packet[len++] = (_impl(_this)->m_segment->m_addr >> 16) & 0xFFFF;
    packet[len++] = (_impl(_this)->m_segment->m_addr) & 0xFFFF;
    packet[len++] = _impl(_this)->m_total_frame;
    packet[len++] = _impl(_this)->m_segment->m_index;
    packet[len++] = _impl(_this)->m_segment->m_is_last;

    return len;
}

static int32_t sm_modbus_rtu_build_fw_info_packet(sm_boot_output_if_t* _this, uint16_t* _buf){
    uint16_t* packet = _buf;
    int32_t len = 0;

    printf("Send fw info size %d\n",_impl(_this)->m_fw_signature.m_size);
    uint32_t temp_size = 97116;

    packet[len] = BSS_MODBUS_FILE_TYPE_FW;
    packet[len] = (packet[len] << 8) | (_impl(_this)->m_fw_signature.m_size & 0xFF);
    len++;

    packet[len++] = _impl(_this)->m_fw_signature.m_size >> 8 ;
    packet[len++] = _impl(_this)->m_fw_signature.m_crc;
    packet[len++] = (_impl(_this)->m_fw_signature.m_addr >> 16) & 0xFFFF;
    packet[len++] = (_impl(_this)->m_fw_signature.m_addr) & 0xFFFF;
    packet[len++] = (((uint16_t)_impl(_this)->m_fw_signature.m_version[2]) << 8) | (uint16_t)(_impl(_this)->m_fw_signature.m_version[2]);
    packet[len++] = (uint16_t) _impl(_this)->m_fw_signature.m_version[0];
    return len;
}

static STATUS_OTA sm_modbus_get_slave_ota_status(sm_boot_output_if_t* _this){
    int32_t ret = -1;
    uint16_t regs[8] = {0,};
    ret = _impl(_this)->m_modbus_read_holding_regs_fn(_impl(_this)->m_modbus_master,
                                                      _impl(_this)->m_src_id + 1,
                                                      MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_OTA_STATUS),
                                                      1,
                                                      regs);
    if (ret == MODBUS_ERROR_NONE){
        printf("Slave ota status is %d\n", regs[0]);
        return regs[0];
    }
    printf("Cant get slave ota status\n");
    return OTA_STATUS_UNKNOWN;
}

int32_t sm_boot_modbus_rtu_process(sm_boot_output_if_t *_this) {
    uint8_t success = 0;
    int len = -1;
    int32_t ret = -1;
    int8_t slaveStatus;
    uint16_t regs[BSS_MODBUS_FILE_MAX_LEN + 1] = {0,};
    if (!_this) {
        return -1;
    }
    switch (_impl(_this)->m_state) {
        case UPGRADE_STATE_IDLE:
            break;
        case UPGRADE_STATE_SEND_FW_INFO:
            if (elapsed_timer_get_remain(&_impl(_this)->m_timeout)) {
                break;
            }
            if (sm_modbus_get_slave_ota_status(_this) == OTA_STATUS_FINISHED) {
                success = true;
                _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&success,
                                                                               _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
                break;
            } else if (sm_modbus_get_slave_ota_status(_this) == OTA_STATUS_FAILURE) {
                success = false;
                _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&success,
                                                                               _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
                break;
            }
            elapsed_timer_reset(&_impl(_this)->m_timeout);
            break;
        case UPGRADE_STATE_REQUEST_UPGRADE:
            if (elapsed_timer_get_remain(&_impl(_this)->m_timeout)) {
                break;
            }
            slaveStatus = sm_modbus_get_slave_ota_status(_this);
            if (slaveStatus == OTA_STATUS_IDLE &&
                _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback) {
                success = true;
                printf("MODBUS OUT: Slave and BP is ready to upgrade\n");
                _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&success,
                                                                               _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);
            }else if(slaveStatus == OTA_STATUS_FAILURE || slaveStatus == OTA_STATUS_FINISHED){
                _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_callback(&success,
                                                                                 _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_arg);
                break;
            }
            elapsed_timer_reset(&_impl(_this)->m_timeout);
            break;
        case UPGRADE_STATE_SEND_SEG_INFO:
            if (elapsed_timer_get_remain(&_impl(_this)->m_timeout)) {
                break;
            }

            slaveStatus = sm_modbus_get_slave_ota_status(_this);

            if (slaveStatus == OTA_STATUS_BUSY) {
                printf("Slave is not idle to receive new segment yet\n");
                elapsed_timer_reset(&_impl(_this)->m_timeout);
                break;
            }else if(slaveStatus == OTA_STATUS_FAILURE || slaveStatus == OTA_STATUS_FINISHED){
                _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_callback(&success,
                                                                               _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_arg);
                break;
            }

            len = sm_modbus_rtu_build_segment_info_packet(_this, regs);
            ret = _impl(_this)->m_modbus_write_mul_regs_fn(_impl(_this)->m_modbus_master,
                                                           _impl(_this)->m_src_id + 1,
                                                           MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_FILE_TYPE),
                                                           len,
                                                           regs);
            if (ret != MODBUS_ERROR_NONE) {
                elapsed_timer_reset(&_impl(_this)->m_timeout);
                break;
            }
            _impl(_this)->m_state = UPGRADE_STATE_SEND_SEG_DATA;
            _impl(_this)->m_frame_index = 0;
            printf("Send Segment info SUCCESS, prepare send segment data\n");
            break;
        case UPGRADE_STATE_SEND_SEG_DATA:
            len = sm_modbus_rtu_build_segment_data_packet(_this, regs);
            ret = _impl(_this)->m_modbus_write_mul_regs_fn(_impl(_this)->m_modbus_master,
                                                           _impl(_this)->m_src_id + 1,
                                                           MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_FILE_FRAME_INDEX),
                                                           len,
                                                           regs);
            if (ret != MODBUS_ERROR_NONE) {
                if (_impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback) {
                    success = false;
                    _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success,
                                                                                 _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);
                }
                return ret;
            }
            _impl(_this)->m_frame_index++;
            if (_impl(_this)->m_frame_index >= _impl(_this)->m_total_frame) {
                if (_impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback) {
                    success = true;
                    _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success,
                                                                                 _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);
                }
            }
            break;
        default:
            _impl(_this)->m_state = UPGRADE_STATE_IDLE;
    }
    return 0;
}

int32_t sm_boot_modbus_rtu_init(sm_boot_output_if_t* _this);
int32_t sm_boot_modbus_rtu_free(sm_boot_output_if_t* _this);
int32_t sm_boot_modbus_rtu_reg_event_callback(sm_boot_output_if_t* _this,
                                              SM_BOOT_SLAVE_EVENT _event,
                                              sm_boot_slave_event_callback_fn_t _fn,
                                              void* _arg);
int32_t sm_boot_modbus_rtu_request_upgrade(sm_boot_output_if_t* _this, int32_t _src_id);
int32_t sm_boot_modbus_rtu_set_fw_info(sm_boot_output_if_t* _this, const sm_fw_signature_t* _fw_info);
int32_t sm_boot_modbus_rtu_set_seg_info(sm_boot_output_if_t* _this, const sm_segment_t* _seg);

static const sm_boot_output_if_proc_t g_boot_output_proc_default = {
        .init = sm_boot_modbus_rtu_init,
        .free = sm_boot_modbus_rtu_free,
        .reg_event_callback = sm_boot_modbus_rtu_reg_event_callback,
        .request_upgrade = sm_boot_modbus_rtu_request_upgrade,
        .set_fw_info = sm_boot_modbus_rtu_set_fw_info,
        .set_seg_fw = sm_boot_modbus_rtu_set_seg_info,
        .process = sm_boot_modbus_rtu_process
};

static sm_boot_modbus_rtu_impl_t g_boot_modbus_rtu_default = {
        .m_base.m_proc = &g_boot_output_proc_default,
        .m_src_id = -1,
        .m_segment = NULL,
        .m_fw_signature = {},
        .m_events = {NULL, NULL, NULL, NULL, NULL}
};

/***
 *
 * @param _this
 * @return
 */
int32_t sm_boot_modbus_rtu_init(sm_boot_output_if_t* _this){
    if(!_this){
        printf( "Boot Modbus RTU is NOT created\n");
        return -1;
    }

    _impl(_this)->m_src_id = -1;

    _impl(_this)->m_segment = NULL;
    _impl(_this)->m_total_frame = 0;
    _impl(_this)->m_frame_index = 0;
    _impl(_this)->m_state = UPGRADE_STATE_IDLE;

    _impl(_this)->m_retry = 0;

    return 0;
}

int32_t sm_boot_modbus_rtu_free(sm_boot_output_if_t* _this){
    _impl(_this)->m_src_id = -1;
    _impl(_this)->m_segment = NULL;
    _impl(_this)->m_total_frame = 0;
    _impl(_this)->m_frame_index = 0;
    _impl(_this)->m_state = UPGRADE_STATE_IDLE;
    _impl(_this)->m_fw_signature;

    return 0;
}

int32_t sm_boot_modbus_rtu_reg_event_callback(sm_boot_output_if_t* _this,
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

int32_t sm_boot_modbus_rtu_request_upgrade(sm_boot_output_if_t* _this, int32_t _src_id){
    if(!_this){
        return -1;
    }

    if(_impl(_this)->m_src_id >= 0 && _impl(_this)->m_src_id != _src_id){
        printf("Modbus RTU is upgrading progress with other Device that Id = %d\n", _impl(_this)->m_src_id);
        return -1;
    }

    _impl(_this)->m_src_id = _src_id;
    _impl(_this)->m_total_frame = 0;
    _impl(_this)->m_frame_index = 0;

    printf("MODBUS OUT: Request upgrade firmware, Source ID: %d\n", _src_id);

    uint16_t regs[BSS_MODBUS_FILE_MAX_LEN] = {0,};
    int32_t len = 0;

    /// Send firmware info first
    len = sm_modbus_rtu_build_fw_info_packet(_this, regs);
    if(len < 0 && _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback){
        bool success = false;
        _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&success,
                                                                       _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);
        return -1;
    }

    int32_t ret = _impl(_this)->m_modbus_write_mul_regs_fn(_impl(_this)->m_modbus_master,
                                                                 _impl(_this)->m_src_id + 1,
                                                                 MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_FILE_TYPE),
                                                                 len,
                                                                 regs);

    if(ret != MODBUS_ERROR_NONE && _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback) {
        bool success = false;
        printf("MODBUS OUT: Send fw info FAILURE\n");
        _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&success,
                                                                       _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);
        return 0;
    }
    printf("MODBUS OUT: Send fw info SUCCESS\n");


    /// Set Slave to Upgrading FW for BP.
    len = 0;
    regs[len++] = 0x0101; //Reboot BP and switch Slave to OTA BP State;
    ret = _impl(_this)->m_modbus_write_mul_regs_fn(_impl(_this)->m_modbus_master,
                                                         _impl(_this)->m_src_id + 1,
                                                         MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_BP_REBOOT),
                                                         len,
                                                         regs);

    if(ret != MODBUS_ERROR_NONE && _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback) {
        bool success = false;
        printf("MODBUS OUT: Send REBOOT REQ FAILURE\n");
        _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&success,
                                                                       _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);
        return 0;
    }
    printf("MODBUS OUT: Send REBOOT REQ SUCCESS\n");

    _impl(_this)->m_state = UPGRADE_STATE_REQUEST_UPGRADE;
    elapsed_timer_resetz(&_impl(_this)->m_timeout, STATE_REQUEST_UPGRADE_TIMEOUT);

    return 0;
}

int32_t sm_boot_modbus_rtu_set_fw_info(sm_boot_output_if_t* _this, const sm_fw_signature_t* _fw_info){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_src_id < 0){
        printf("modbus_rtu is NOT active or busy");
        return -1;
    }

    _impl(_this)->m_fw_signature = *_fw_info;
    printf( "New fw with version: %X.%X.%X\n size: %d \n crc: %d\n",
            _fw_info->m_version[0],
            _fw_info->m_version[1],
            _fw_info->m_version[2],
            _fw_info->m_size,
            _fw_info->m_crc);

    _impl(_this)->m_state = UPGRADE_STATE_SEND_FW_INFO;

    elapsed_timer_resetz(&_impl(_this)->m_timeout, STATE_SEND_FW_INFO_TIMEOUT);

    return 0;
}

int32_t sm_boot_modbus_rtu_set_seg_info(sm_boot_output_if_t* _this, const sm_segment_t* _seg){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_src_id < 0){
        printf( "Modbus_rtu is NOT active or busy");
        return -1;
    }

    _impl(_this)->m_segment = _seg;

    _impl(_this)->m_total_frame = _impl(_this)->m_segment->m_size/FILE_SEG_FRAME_SIZE;
    if(_impl(_this)->m_segment->m_size%FILE_SEG_FRAME_SIZE != 0){
        _impl(_this)->m_total_frame++;
    }

    printf("Src ID: %d. Segment info: Size: %d, CRC: 0x%2X, Total frame: %d, Address 0x%2X, Index: %d\n",
           _impl(_this)->m_src_id,
           _seg->m_size,
           _seg->m_crc,
           _impl(_this)->m_total_frame,
           _seg->m_addr,
           _seg->m_index);

    _impl(_this)->m_frame_index = 0;
    elapsed_timer_resetz(&_impl(_this)->m_timeout, STATE_SEND_SEG_INFO_TIMEOUT);
    _impl(_this)->m_state = UPGRADE_STATE_SEND_SEG_INFO;
    _impl(_this)->m_retry = 0;

    return 1;
}

sm_boot_output_if_t* sm_get_modbus_rtu_boot_output(const sm_fw_signature_t* _fw_info,
                                                   void* _mb_master,
                                                   modbus_rtu_write_multiple_registers_fn_t _mb_write_fn,
                                                   modbus_rtu_read_holding_registers_fn_t _mb_read_fn){
    if(!_fw_info || !_mb_write_fn || !_mb_master || !_mb_read_fn){
        return NULL;
    }

    g_boot_modbus_rtu_default.m_base.m_proc = &g_boot_output_proc_default;
    g_boot_modbus_rtu_default.m_src_id = -1;
    g_boot_modbus_rtu_default.m_segment = NULL;
    g_boot_modbus_rtu_default.m_total_frame = 0;
    g_boot_modbus_rtu_default.m_frame_index = 0;

    sm_fw_signature_clone(&g_boot_modbus_rtu_default.m_fw_signature, _fw_info);

    printf("Modbus output create with fw input size %d\n", g_boot_modbus_rtu_default.m_fw_signature.m_size);

    g_boot_modbus_rtu_default.m_modbus_master = _mb_master;
    g_boot_modbus_rtu_default.m_modbus_write_mul_regs_fn = _mb_write_fn;
    g_boot_modbus_rtu_default.m_modbus_read_holding_regs_fn = _mb_read_fn;

    elapsed_timer_resetz(&g_boot_modbus_rtu_default.m_timeout, STATE_REQUEST_UPGRADE_TIMEOUT);

    for(int index = 0; index < SM_BOOT_SLAVE_EVENT_NUMBER; index++){
        g_boot_modbus_rtu_default.m_events[index].m_callback = NULL;
        g_boot_modbus_rtu_default.m_events[index].m_arg = NULL;
    }

    return &g_boot_modbus_rtu_default.m_base;
}