//
// Created by vnbk on 28/03/2023.
//
#include <stdio.h>
#include "sm_boot_slave.h"
#include "sm_elapsed_timer.h"

#include "app_co_init.h"

#include "sm_boot_impl.h"

#include "sm_logger.h"

#define TAG "sm_canopen_boot_output"

#define _impl(p) ((sm_boot_canopen_impl_t*)(p))

extern CO CO_DEVICE;
extern CO_OD *p_co_od;

typedef enum {
    UPGRADE_STATE_IDLE,
    UPGRADE_STATE_REQUEST_UPGRADE,
    UPGRADE_STATE_SEND_FW_INFO,
    UPGRADE_STATE_SEND_SEG,
    UPGRADE_STATE_NUMBER
}CANOPEN_UPGRADE_PROCESS_STATE;

#define CANOPEN_SEND_FW_SIZE_STEP       0
#define CANOPEN_SEND_FW_CRC_STEP        1
#define CANOPEN_SEND_FW_VERSION_STEP    2

#define CANOPEN_SEND_SEG_ADDR_STEP      0
#define CANOPEN_SEND_SEG_DATA_STEP      1
#define CANOPEN_SEND_SEG_CRC_STEP       2

#define BOOT_ST_LOADING_SERVER          2

#define PERIOD_REQUEST_UPGRADE_TIME     100

#define WAIT_DEV_PREPARE_UPGRADE        1000

typedef struct {
    sm_boot_slave_event_callback_fn_t m_callback;
    void* m_arg;
}sm_boot_output_event_t;

typedef struct {
    sm_boot_output_if_t m_base;
    int32_t m_src_node_id;

    sm_boot_output_event_t m_events[SM_BOOT_SLAVE_EVENT_NUMBER];

    const sm_fw_signature_t* m_fw_signature;
    const sm_segment_t* m_segment;

    CANOPEN_UPGRADE_PROCESS_STATE m_state;
    int32_t m_step;

    elapsed_timer_t m_timeout;
    uint8_t m_upgrade_request;
}sm_boot_canopen_impl_t;

static int32_t sm_boot_canopen_send_fw_version(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;

    if(CO_SDO_get_status(p_sdo) != CO_SDO_RT_idle || _impl(_this)->m_step != CANOPEN_SEND_FW_VERSION_STEP){
        return -1;
    }

    CO_Sub_Object fw_version_obj = {
            .p_data = (void*)&_impl(_this)->m_fw_signature->m_version,
            .attr   = ODA_SDO_RW,
            .len    = 3,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _impl(_this)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_FW_VERSION_SUB_INDEX,
                                &fw_version_obj, 1000);
    printf( "Send fw Version\n");
    return 0;
}

static int32_t sm_boot_canopen_send_fw_size(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;

    if(CO_SDO_get_status(p_sdo) != CO_SDO_RT_idle || _impl(_this)->m_step != CANOPEN_SEND_FW_SIZE_STEP){
        return -1;
    }

    CO_Sub_Object fw_size_obj = {
            .p_data = (void*)&_impl(_this)->m_fw_signature->m_size,
            .attr   = ODA_SDO_RW,
            .len    = 4,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _impl(_this)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_FW_SIZE_SUB_INDEX,
                                &fw_size_obj, 1000);
    printf( "Send fw SIZE\n");
    return 0;
}

static int32_t sm_boot_canopen_send_fw_crc(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;

    if(CO_SDO_get_status(p_sdo) != CO_SDO_RT_idle || _impl(_this)->m_step != CANOPEN_SEND_FW_CRC_STEP){
        return -1;
    }

    CO_Sub_Object fw_crc_obj = {
            .p_data = (void*)&_impl(_this)->m_fw_signature->m_crc,
            .attr   = ODA_SDO_RW,
            .len    = 2,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _impl(_this)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_FW_CRC_SUB_INDEX,
                                &fw_crc_obj, 1000);
    printf( "Send fw CRC\n");
    return 0;
}
static int32_t sm_boot_canopen_send_seg_addr(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;


    if(CO_SDO_get_status(p_sdo) != CO_SDO_RT_idle || _impl(_this)->m_step != CANOPEN_SEND_SEG_ADDR_STEP){
//        printf( "SDO Status busy");
        return -1;
    }

    CO_Sub_Object seg_addr_obj = {
            .p_data = (void*)&_impl(_this)->m_segment->m_addr,
            .attr   = ODA_SDO_RW,
            .len    = 4,
            .p_ext  = NULL
    };

    CO_SDOclient_start_download(p_sdo,
                                _impl(_this)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_SEG_ADDR_SUB_INDEX,
                                &seg_addr_obj, 5000);

    printf( "Send SEG address 0x%x \n", _impl(_this)->m_segment->m_addr);
    return 0;
}
static int32_t sm_boot_canopen_send_seg_data(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;

    if(CO_SDO_get_status(p_sdo) != CO_SDO_RT_idle || _impl(_this)->m_step != CANOPEN_SEND_SEG_DATA_STEP){
        return -1;
    }

    CO_Sub_Object seg_data_obj = {
            .p_data = (void*)_impl(_this)->m_segment->m_data,
            .attr   = ODA_SDO_RW,
            .len    = _impl(_this)->m_segment->m_length,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _impl(_this)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_SEG_DATA_SUB_INDEX,
                                &seg_data_obj, 5000);
    printf( "Send SEG data %d byte\n", _impl(_this)->m_segment->m_length);
    return 0;
}
static int32_t sm_boot_canopen_send_seg_crc(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;

    if(CO_SDO_get_status(p_sdo) != CO_SDO_RT_idle || _impl(_this)->m_step != CANOPEN_SEND_SEG_CRC_STEP){
        return -1;
    }

    CO_Sub_Object seg_crc_obj = {
            .p_data = (void*)&_impl(_this)->m_segment->m_crc,
            .attr   = ODA_SDO_RW,
            .len    = 2,
            .p_ext  = NULL
    };
    CO_SDOclient_start_download(p_sdo,
                                _impl(_this)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_SEG_CRC_SUB_INDEX,
                                &seg_crc_obj, 5000);

    printf( "Send SEG CRC: 0x%x\n", _impl(_this)->m_segment->m_crc);
    return 0;
}

static int32_t sm_boot_canopen_send_request_upgrade(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    if(CO_SDO_get_status(p_sdo) == CO_SDO_RT_busy){
//        printf( "Can bus busy");
        return -1;
    }

    CO_Sub_Object request_upgrade_obj = {
            .p_data = &_impl(_this)->m_upgrade_request,
            .attr   = ODA_SDO_RW,
            .len    = 1,
            .p_ext  = NULL
    };

    CO_SDOclient_start_download(p_sdo,
                                _impl(_this)->m_src_node_id,
                                SDO_BOOTLOADER_INDEX,
                                SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX,
                                &request_upgrade_obj, PERIOD_REQUEST_UPGRADE_TIME);


    return 0;
}

int32_t sm_boot_canopen_response_handle(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    if(!_this || !p_sdo){
        return -1;
    }

    CO_SDO_return_t sdo_status = CO_SDO_get_status(p_sdo);

    if(sdo_status == CO_SDO_RT_abort){
        LOG_ERR("CO boot", "SDO status: Abort, Tx_abort: 0x%2X, Rx_Abort: 0x%2X \n", p_sdo->tx_abort_code, p_sdo->rx_abort_code);
        if(_impl(_this)->m_step < 0){
            CO_SDO_reset_status(p_sdo);
            return 0;
        }
        uint8_t success = false;

        if(_impl(_this)->m_state == UPGRADE_STATE_SEND_SEG){
            _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success,
                                                                         _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);
        }else if(_impl(_this)->m_state == UPGRADE_STATE_SEND_FW_INFO){
            _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&success,
                                                                             _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
        }
        _impl(_this)->m_step = -1;
        CO_SDO_reset_status(p_sdo);
    }else if(sdo_status == CO_SDO_RT_success){
//        printf( "%ld - SDO send SUCCESS. State: %d, Step: %d \n", get_tick_count(), _impl(_this)->m_state, _impl(_this)->m_step);

        if(_impl(_this)->m_state != UPGRADE_STATE_REQUEST_UPGRADE){
            _impl(_this)->m_step++;
        }else{
            elapsed_timer_resetz(&_impl(_this)->m_timeout, WAIT_DEV_PREPARE_UPGRADE);
        }

        if(_impl(_this)->m_step >= 3){
            uint8_t success = true;
            if(_impl(_this)->m_state == UPGRADE_STATE_SEND_SEG){
                /// Uncomment for Bootloader simulator on Desktop
                /*  _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success, _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);
                  _impl(_this)->m_step = -1;*/
            } else if(_impl(_this)->m_state == UPGRADE_STATE_SEND_FW_INFO){
                _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_callback(&success, _impl(_this)->m_events[SM_BOOT_SLAVE_CONFIRM_FW_INFO].m_arg);
                _impl(_this)->m_step = -1;
            }
        }
        CO_SDO_reset_status(p_sdo);
    }
    return 0;
}

int32_t sm_boot_canopen_process(sm_boot_output_if_t* _this){
    CO_SDO* p_sdo = &CO_DEVICE.sdo_client;
    if(!_this || !p_sdo){
        return -1;
    }

    CO_SDO_return_t sdo_status = CO_SDO_get_status(p_sdo);

    switch (_impl(_this)->m_state) {
        case UPGRADE_STATE_IDLE:
            break;
        case UPGRADE_STATE_REQUEST_UPGRADE:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_timeout)){
                sm_boot_canopen_send_request_upgrade(_this);
                elapsed_timer_resetz(&_impl(_this)->m_timeout, PERIOD_REQUEST_UPGRADE_TIME);
            }
            break;

        case UPGRADE_STATE_SEND_FW_INFO:
            if(_impl(_this)->m_step == CANOPEN_SEND_FW_SIZE_STEP){
                sm_boot_canopen_send_fw_size(_this);
            }else if(_impl(_this)->m_step == CANOPEN_SEND_FW_CRC_STEP){
                sm_boot_canopen_send_fw_crc(_this);
            }else if(_impl(_this)->m_step == CANOPEN_SEND_FW_VERSION_STEP){
                sm_boot_canopen_send_fw_version(_this);
            }

            if(_impl(_this)->m_step < 0 && (sdo_status == CO_SDO_RT_idle)){
                _impl(_this)->m_step = CANOPEN_SEND_FW_SIZE_STEP;
            }
            break;

        case UPGRADE_STATE_SEND_SEG:
            if(_impl(_this)->m_step == CANOPEN_SEND_SEG_ADDR_STEP){
                sm_boot_canopen_send_seg_addr(_this);
            }else if(_impl(_this)->m_step == CANOPEN_SEND_SEG_DATA_STEP){
                sm_boot_canopen_send_seg_data(_this);
            }else if(_impl(_this)->m_step == CANOPEN_SEND_SEG_CRC_STEP){
                sm_boot_canopen_send_seg_crc(_this);
            }

            if(_impl(_this)->m_step < 0 && (sdo_status == CO_SDO_RT_idle)){
                _impl(_this)->m_step = CANOPEN_SEND_SEG_ADDR_STEP;
            }
            break;

        default:
            _impl(_this)->m_state = UPGRADE_STATE_IDLE;
    }
    return sm_boot_canopen_response_handle(_this);
}

/***
 *
 * @param _this
 * @return
 */
int32_t sm_boot_canopen_init(sm_boot_output_if_t* _this){
//    app_co_init();
    return 0;
}

int32_t sm_boot_canopen_free(sm_boot_output_if_t* _this){
//    app_co_free();

    _impl(_this)->m_step = -1;
    _impl(_this)->m_state = UPGRADE_STATE_IDLE;
    _impl(_this)->m_src_node_id = -1;
    _impl(_this)->m_segment = false;
    _impl(_this)->m_fw_signature = NULL;
    return 0;
}

int32_t sm_boot_canopen_reg_event_callback(sm_boot_output_if_t* _this,
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

int32_t sm_boot_canopen_request_upgrade(sm_boot_output_if_t* _this, int32_t _src_node_id){
    if(!_this){
        return -1;
    }

    if(_impl(_this)->m_src_node_id >= 0 && _impl(_this)->m_src_node_id != _src_node_id){
        //printf( "CanOpen is upgrading progress with other Device that Node Id = %d", _impl(_this)->m_src_node_id);
        printf("CanOpen is upgrading progress with other Device that Node Id = %d\n", _impl(_this)->m_src_node_id);
        return -1;
    }

    _impl(_this)->m_src_node_id = _src_node_id;
    _impl(_this)->m_state = UPGRADE_STATE_REQUEST_UPGRADE;

    printf( "Request upgrade firmware, Source NODE ID: %d\n", _src_node_id);

    return 0;
}

int32_t sm_boot_canopen_set_fw_info(sm_boot_output_if_t* _this, const sm_fw_signature_t* _fw_info){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_src_node_id < 0){
        //printf( "CanOpen is NOT active or busy");
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
    _impl(_this)->m_step = -1;

    return 0;
}

int32_t sm_boot_canopen_set_seg_fw(sm_boot_output_if_t* _this, const sm_segment_t* _seg){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_src_node_id < 0){
        //printf( "CanOpen is NOT active or busy");
        return -1;
    }

    _impl(_this)->m_segment = _seg;
    _impl(_this)->m_state = UPGRADE_STATE_SEND_SEG;
    _impl(_this)->m_step = -1;

    printf("Src ID: %d. Segment info: Size: %d, CRC: 0x%2X, Address 0x%x, Index: %d. Segment %s: CRC: 0x%02X\n",
           _impl(_this)->m_src_node_id,
           _seg->m_size,
           _seg->m_crc,
           _seg->m_addr,
           _seg->m_index,
           sm_seg_is_valid(_seg) ? "VALID" : "INVALID", _seg->m_crc);

    return 0;
}


static const sm_boot_output_if_proc_t g_boot_output_proc_default = {
        .init = sm_boot_canopen_init,
        .free = sm_boot_canopen_free,
        .reg_event_callback = sm_boot_canopen_reg_event_callback,
        .request_upgrade = sm_boot_canopen_request_upgrade,
        .set_fw_info = sm_boot_canopen_set_fw_info,
        .set_seg_fw = sm_boot_canopen_set_seg_fw,
        .process = sm_boot_canopen_process
};

static sm_boot_canopen_impl_t g_boot_canopen_default = {
        .m_base.m_proc = &g_boot_output_proc_default,
        .m_step = -1,
        .m_src_node_id = -1,
        .m_state = UPGRADE_STATE_IDLE,
        .m_segment = NULL,
        .m_fw_signature = NULL,
        .m_upgrade_request = true,
        .m_events = {NULL, NULL, NULL, NULL, NULL}
};

static uint8_t g_response_state = 0;
static uint8_t g_response_ext_boot = 0;

static CO_Sub_Object_Ext_Confirm_Func_t sm_boot_canopen_response_state_handle(void){
    printf("changer status to LOADING_SERVER\n");
    if(g_response_state == BOOT_ST_LOADING_SERVER && g_boot_canopen_default.m_state == UPGRADE_STATE_SEND_SEG){
        if(g_boot_canopen_default.m_step >= 2) {
            printf("Sync slave state: %d\n\n", g_response_state);
            uint8_t success = true;
            g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_callback(&success,
                                                                                  g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_SEG].m_arg);

            g_boot_canopen_default.m_step = -1;
        }
        return CO_EXT_CONFIRM_success;
    }

    if(g_boot_canopen_default.m_state == UPGRADE_STATE_REQUEST_UPGRADE){
        uint8_t ready = true;
        g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_callback(&ready,
                                                                                g_boot_canopen_default.m_events[SM_BOOT_SLAVE_CONFIRM_READY].m_arg);

        printf("Confirm slave ready: %d\n", g_response_state);
        g_boot_canopen_default.m_state = UPGRADE_STATE_IDLE;
    }
    return CO_EXT_CONFIRM_success;
}

static CO_Sub_Object_Ext_Confirm_Func_t sm_boot_canopen_response_ext_boot_handle(void){
    printf("Confirm external request slave ready: %d\n", g_response_ext_boot);
    return CO_EXT_CONFIRM_success;
}

sm_boot_output_if_t* sm_get_co_boot_output(){
    struct CO_Object_t* fw_obj = NULL;

    for(int index = 0; index < p_co_od->number; index++){
        if(p_co_od->list[index].index == SDO_BOOTLOADER_INDEX){
            fw_obj = &p_co_od->list[index];
            break;
        }
    }
    fw_obj->subs[SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX].p_data = &g_response_ext_boot;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX].p_ext->p_shadow_data = &g_response_ext_boot;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX].p_ext->confirm_func = sm_boot_canopen_response_ext_boot_handle;

    fw_obj->subs[SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX].p_data = &g_response_state;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX].p_ext->p_shadow_data = &g_response_state;
    fw_obj->subs[SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX].p_ext->confirm_func = sm_boot_canopen_response_state_handle;

    CO_set_node_id(&CO_DEVICE, BOOT_MASTER_NODE_ID);

    g_boot_canopen_default.m_step = -1;
    g_boot_canopen_default.m_state = UPGRADE_STATE_IDLE;
    g_boot_canopen_default.m_src_node_id = -1;
    g_boot_canopen_default.m_segment = false;
    g_boot_canopen_default.m_fw_signature = NULL;

    elapsed_timer_resetz(&g_boot_canopen_default.m_timeout, PERIOD_REQUEST_UPGRADE_TIME);

    return &g_boot_canopen_default.m_base;
}
