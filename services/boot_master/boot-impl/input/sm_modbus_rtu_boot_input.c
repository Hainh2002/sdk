//
// Created by vnbk on 23/11/2023.
//
//
// Created by vnbk on 28/03/2023.
//
#include <stdio.h>
#include "sm_boot_impl.h"
#include "sm_logger.h"
#include "sm_crc.h"
#include "sm_mb_slave_impl.h"

#define TAG "sm_modbus_rtu_boot_input"

#define QUEUE_SIZE  100

#define MODBUS_RTU_ENCODE_ID(code,reg)               ((((uint32_t)code) << 16) | (reg+1))
#define MODBUS_RTU_DECODE_FUNCTION_CODE(value)       ((uint8_t)((value)>>16 & 0xFF))
#define MODBUS_RTU_DECODE_REGISTER(value)            ((uint16_t)(((value) & 0xFFFF) - 1))

/// DECODE register Value;
#define MODBUS_RTU_REG_GET_HIGH_BYTE(value)         (((value) >> 8) & 0xFF)
#define MODBUS_RTU_REG_GET_LOW_BYTE(value)          ((value) & 0xFF)

#define MODBUS_REQUEST_UPGRADE_BP       0x30007

#define MODBUS_FILE_TYPE                0x30020
#define MODBUS_FILE_SIZE                0x30021
#define MODBUS_FILE_CRC                 0x30022
#define MODBUS_FILE_ADDR_0              0x30023
#define MODBUS_FILE_ADDR_1              0x30024
#define MODBUS_FILE_TOTAL               0x30025
#define MODBUS_FILE_ADDITIONAL          0x30026
#define MODBUS_FILE_CMD                 0x30028
#define MODBUS_FILE_FRAME_INDEX         0x30029
#define MODBUS_FILE_FRAME_DATA          0x30030

#define MODBUS_FILE_TYPE_INDEX      0
#define MODBUS_FILE_SIZE_INDEX      1
#define MODBUS_FILE_CRC_INDEX       2
#define MODBUS_FILE_ADDR_INDEX      3
#define MODBUS_FILE_TOTAL_FRAME_INDEX   5
#define MODBUS_FILE_ADDITIONAL_INDEX    6

#define MODBUS_FILE_FRAME_INDEX_INDEX   0
#define MODBUS_FILE_FRAME_DATA_INDEX    1

#define MODBUS_FILE_TYPE_SEG_INFO       1
#define MODBUS_FILE_TYPE_FW_INFO        0

#define SEG_STATE_IDLE                  0
#define SEG_STATE_LOADING_FROM_HOST     1
#define SEG_STATE_FULL                  2
#define SEG_STATE_WAITING_UPLOAD        3

#define TAG "sm_modbus_rtu_boot_input"

typedef struct {
    uint8_t m_address;
    uint16_t m_seg_total_frame;
    uint16_t m_seg_frame_index;

    sm_segment_t m_seg;
    uint8_t m_seg_state;

    sm_fw_signature_t m_fw_signature;
    uint16_t m_seg_total;

    sm_mb_slave_t* m_mb_instance;
    void* m_arg;
}sm_modbus_rtu_boot_input_t;

static sm_modbus_rtu_boot_input_t g_modbus_rtu_boot = {
        .m_seg_total = 0,
        .m_seg_total_frame = 0,
        .m_seg_frame_index = 0,
        .m_address = 0
};

MODBUS_ERROR_CODE sm_write_single_register_callback(uint16_t _address,
                                             uint16_t _value,
                                             uint8_t _unit_id,
                                             void* _arg){
    if(_address == MODBUS_RTU_DECODE_REGISTER(MODBUS_REQUEST_UPGRADE_BP)){
        if(_value == 1){
            LOG_DBG(TAG, "BP is ready to upgrade firmware xxxxxx");
        }
    }
    return MODBUS_ERROR_NONE;
}

MODBUS_ERROR_CODE sm_read_holding_registers_callback(uint16_t _address,
                                                  uint16_t _quantity,
                                                  uint16_t* _registers_out,
                                                  uint8_t _unit_id,
                                                  void* _arg){
    static int count = 0;
//    LOG_DBG(TAG, "Read holding registers m_cb");
    if(_address == MODBUS_RTU_DECODE_REGISTER(MODBUS_REQUEST_UPGRADE_BP)){
        count++;
        if(count > 5){
            _registers_out[0] = 1;
            LOG_WRN(TAG, "BP is ready to upgrade firmware");
            count = 0;
        }

    }
    return MODBUS_ERROR_NONE;
}

MODBUS_ERROR_CODE sm_write_multiple_registers_callback(uint16_t _address,
                                                uint16_t _quantity,
                                                const uint16_t* _registers,
                                                uint8_t _unit_id,
                                                void* _arg){
    sm_modbus_rtu_boot_input_t* modbus_rtu_boot = &g_modbus_rtu_boot;
//    LOG_DBG(TAG, "Write multiple registers m_cb");

    if(_address == MODBUS_RTU_DECODE_REGISTER(MODBUS_FILE_TYPE)){
        uint8_t  type = (_registers[MODBUS_FILE_TYPE_INDEX] >> 8) & 0xFF;
        int32_t size =  (uint32_t)_registers[MODBUS_FILE_SIZE_INDEX] << 8 | (_registers[MODBUS_FILE_TYPE_INDEX] & 0xFF) ;
        uint16_t crc  = _registers[MODBUS_FILE_CRC_INDEX];
        int32_t addr = ((uint32_t)_registers[MODBUS_FILE_ADDR_INDEX] << 16) | (uint32_t)_registers[MODBUS_FILE_ADDR_INDEX + 1];
        uint16_t total = _registers[MODBUS_FILE_TOTAL_FRAME_INDEX];

        if(type == MODBUS_FILE_TYPE_FW_INFO){
            modbus_rtu_boot->m_fw_signature.m_size = size;
            modbus_rtu_boot->m_fw_signature.m_crc = crc;
            modbus_rtu_boot->m_fw_signature.m_addr = addr;
            modbus_rtu_boot->m_seg_total = total;
            
            memcpy(modbus_rtu_boot->m_fw_signature.m_version, (uint8_t*)&_registers[MODBUS_FILE_ADDITIONAL_INDEX], 3);

            LOG_INF(TAG, "Received new FW info from HOST, Size: %d, CRC: 0x%2X, Total frame: %d, Address: 0x%2X", size, crc, total, addr);
        }else{
            sm_segment_t* seg = &modbus_rtu_boot->m_seg;
            seg->m_size = size;
            seg->m_crc = crc;
            seg->m_addr = addr;
            modbus_rtu_boot->m_seg_total_frame = total;
            seg->m_index = _registers[MODBUS_FILE_ADDITIONAL_INDEX];
            seg->m_is_last = _registers[MODBUS_FILE_ADDITIONAL_INDEX+1];
            seg->m_length = 0;
            LOG_INF(TAG, "Received new segment info from HOST, Size: %d, CRC: 0x%2X, Total frame: %d, Address: 0x%2X, Index: %d",
                    size, crc, total, addr, _registers[MODBUS_FILE_ADDITIONAL_INDEX]);

            modbus_rtu_boot->m_seg_state = SEG_STATE_LOADING_FROM_HOST;
        }
    }else if(_address == MODBUS_RTU_DECODE_REGISTER(MODBUS_FILE_FRAME_INDEX)){
        if(modbus_rtu_boot->m_seg_state != SEG_STATE_LOADING_FROM_HOST){
            LOG_ERR(TAG, "State INVALID to received data");
            return MODBUS_ERROR_INVALID_RESPONSE;
        }

        modbus_rtu_boot->m_seg_frame_index = _registers[MODBUS_FILE_FRAME_INDEX_INDEX];
        sm_seg_fill_data(&modbus_rtu_boot->m_seg,
                         (const uint8_t*)(&_registers[MODBUS_FILE_FRAME_DATA_INDEX]),
                         (_quantity-1)*2);

        if(modbus_rtu_boot->m_seg_total_frame <= (modbus_rtu_boot->m_seg_frame_index+1)) {
            LOG_INF(TAG, "Received frame of segment completed. CRC = 0x%2X ~ = 0x%2X",
                    modbus_rtu_boot->m_seg.m_crc,
                    sm_CRC_CalculateCRC16(modbus_rtu_boot->m_seg.m_data, 1024));

            modbus_rtu_boot->m_seg_state = SEG_STATE_FULL;
        }
    }
    return MODBUS_ERROR_NONE;
}

static sm_mb_slave_cb_t g_modbus_rtu_callback = {
        .read_discrete_inputs = NULL,
        .read_input_registers = NULL,
        .read_holding_registers = sm_read_holding_registers_callback,
        .write_single_coil = NULL,
        .write_multiple_coils = NULL,
        .write_single_register = sm_write_single_register_callback,
        .write_multiple_registers = sm_write_multiple_registers_callback
};

int32_t sm_modbus_rtu_init(){
    g_modbus_rtu_boot.m_seg_total = 0;
    g_modbus_rtu_boot.m_seg_total_frame = 0;
    g_modbus_rtu_boot.m_seg_frame_index = 0;
    g_modbus_rtu_boot.m_address = 0;

    g_modbus_rtu_boot.m_seg_total = g_modbus_rtu_boot.m_fw_signature.m_size/SM_SEGMENT_FW_SIZE;
    if(g_modbus_rtu_boot.m_seg_total % SM_SEGMENT_FW_SIZE != 0){
        g_modbus_rtu_boot.m_seg_total += 1;
    }
    LOG_INF(TAG, "Total segment in new firmware: %d", g_modbus_rtu_boot.m_seg_total);

    MODBUS_ERROR_CODE err = g_modbus_rtu_boot.m_mb_instance->m_proc->reg_function_cb(g_modbus_rtu_boot.m_mb_instance,
                                                                                     &g_modbus_rtu_callback);

    if(err != MODBUS_ERROR_NONE){
        return -1;
    }
    return 0;
}

int32_t sm_modbus_rtu_free(){
    g_modbus_rtu_boot.m_seg_total = 0;
    g_modbus_rtu_boot.m_seg_total_frame = 0;
    g_modbus_rtu_boot.m_seg_frame_index = 0;
    g_modbus_rtu_boot.m_address = 0;
    sm_fw_signature_reset(&g_modbus_rtu_boot.m_fw_signature);

    return 0;
}

int32_t sm_modbus_rtu_get_fw_info(sm_fw_signature_t* _fw){
    if(!_fw){
        return -1;
    }
    sm_fw_signature_clone(_fw, &g_modbus_rtu_boot.m_fw_signature);
    return 0;
}

int32_t sm_modbus_rtu_get_total_seg(){
    return (g_modbus_rtu_boot.m_seg_total);
}

int32_t sm_modbus_rtu_get_seg_fw(sm_segment_t* _seg){
    if(!_seg){
        return -1;
    }

    if(g_modbus_rtu_boot.m_seg_state == SEG_STATE_FULL){
        sm_seg_clone(_seg, &g_modbus_rtu_boot.m_seg);
        g_modbus_rtu_boot.m_seg_state = SEG_STATE_WAITING_UPLOAD;
        return 1;
    }
    return 0;
}

int32_t sm_seg_upgrade_completed(int32_t _seg_index){
    if(g_modbus_rtu_boot.m_seg_state == SEG_STATE_WAITING_UPLOAD){
        g_modbus_rtu_boot.m_seg_state = SEG_STATE_IDLE;
    }
    return 0;
}

int32_t sm_modbus_rtu_process(){
    if(g_modbus_rtu_boot.m_seg_state == SEG_STATE_IDLE || g_modbus_rtu_boot.m_seg_state == SEG_STATE_LOADING_FROM_HOST){
//        LOG_WRN(TAG, "Queue is FULL, wait a second....");
        return g_modbus_rtu_boot.m_mb_instance->m_proc->polling(g_modbus_rtu_boot.m_mb_instance);
    }
    return 0;
}

sm_boot_input_if_t g_boot_input_proc = {
        .init = sm_modbus_rtu_init,
        .free = sm_modbus_rtu_free,
        .get_total_seg = sm_modbus_rtu_get_total_seg,
        .get_fw_info = sm_modbus_rtu_get_fw_info,
        .get_seg_fw = sm_modbus_rtu_get_seg_fw,
        .seg_upgrade_completed = sm_seg_upgrade_completed,
        .process = sm_modbus_rtu_process
};

sm_boot_input_if_t* sm_get_modbus_rtu_boot_input(uint8_t _address,
                                                 const sm_fw_signature_t* _fw_info,
                                                 sm_mb_slave_t* _mb,
                                                 void* _arg){
    g_modbus_rtu_boot.m_address = _address;

    g_modbus_rtu_boot.m_mb_instance = _mb;
    g_modbus_rtu_boot.m_arg = _arg;

    sm_fw_signature_clone(&g_modbus_rtu_boot.m_fw_signature, _fw_info);

    return &g_boot_input_proc;
}