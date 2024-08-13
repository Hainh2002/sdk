#include "sm_bp_co.h"
#include "sm_logger.h"
#include "sm_co_od_common.h"

//#include "sm_bp_auth.h"
#include "sm_co_if.h"
#define MAIN_TAG "sm_bp_co"

#define TAG "SM_BP_CO"

#define SDO_READ_OBJ_TIMEOUT_MS							5000
#define SDO_WRITE_OBJ_TIMEOUT_MS						5000

#define BMS_INDEX										0x2003
#define BMS_SERIAL_NUMBER_OBJECT_SUB_INDEX				0x00
#define BMS_MAIN_SWITCH_SUB_INDEX						0x01
#define SLAVE_ID_NUMBER_OBJECT_SUB_INDEX				0x02

#define BMS_VERSION_INDEX								0x100A
#define BMS_VERSION_SUB_INDEX							0x00

#define BMS_VEHICLE_SN_INDEX							0x2004
#define BMS_MATTED_DEV_SUB_INDEX						0x01

#define SM_DEVICE_BOOT_INDEX                            0x2001
#define SN_DEVICE_REBOOT_SUB_INDEX                      0x07

#define BMS_STATE_CHARGING								3
#define BMS_STATE_DISCHARGING							3
#define BMS_STATE_ONLY_DISCHARGING                      4
#define BMS_STATE_STANDBY                               0

#define BP_NODE_ID_OFFSET                               5

#define BP_VOL_UPPER_THRESHOLD	7000 	//	10*mv
#define BP_VOL_LOWER_THRESHOLD	4000	// 	10*mv

#define BP_CUR_UPPER_THRESHOLD	2000 	//	10*mv
#define BP_CUR_LOWER_THRESHOLD	(-2000)	// 	10*mv


#define _impl(p)        ((sm_bp_co_impl_t*)(p))

typedef struct{
    sm_co_t* m_co;
    sm_co_if_t* m_co_if;

    uint8_t m_sync;
    
    sm_bp_t* m_bps;
    uint8_t m_bp_numb;

    sm_bp_co_update_data_fn_t m_update_fn;
    void* m_update_arg;

    const sm_bp_cmd_t* m_cmd;
    uint8_t m_cmd_temp;
}sm_bp_co_impl_t;

static sm_bp_co_impl_t g_bp_co;

static void sm_bp_co_extract_pdo(sm_bp_co_impl_t* _this, uint32_t _cob_id, uint8_t* _data){
    uint32_t cob_id = _cob_id & 0xFFFFFF80;
    uint8_t node_id = (uint8_t) (_cob_id & 0x7F);
    uint8_t bp_id = node_id - BP_NODE_ID_OFFSET;
    
    if(bp_id >= _this->m_bp_numb){
        return;
    }

    sm_bp_t* bp = &_this->m_bps[bp_id];
    
    switch(cob_id){
        case BP_VOL_CUR_TPDO_COBID:
        	if(CO_getUint16(_data) < BP_VOL_UPPER_THRESHOLD && CO_getUint16(_data) > BP_VOL_LOWER_THRESHOLD){
        	      bp->m_data.m_vol = CO_getUint16(_data);
        	}
        	if((int32_t)CO_getUint16(_data + 2) > BP_CUR_LOWER_THRESHOLD && (int32_t)CO_getUint16(_data + 2) < BP_CUR_UPPER_THRESHOLD){
        		bp->m_data.m_cur = CO_getUint16(_data + 2);
        	}
            bp->m_data.m_soc 	= _data[4];
            bp->m_data.m_state 	= _data[5];
            bp->m_data.m_status	= CO_getUint16(_data + 6);

            bp->m_data_count++;

            break;
        case BP_CELLS_VOL_1_TO_4:
            CO_memcpy((uint8_t*)(bp->m_data.m_cellVols), _data, 8);
            bp->m_data_count++;
            break;
        case BP_CELLS_VOL_5_TO_8:
            CO_memcpy((uint8_t*)(bp->m_data.m_cellVols + 4), _data, 8);
            bp->m_data_count++;
            break;
        case BP_CELLS_VOL_9_TO_12:
            CO_memcpy((uint8_t*)(bp->m_data.m_cellVols + 8), _data, 8);
            bp->m_data_count++;
            break;
        case BP_CELLS_VOL_13_TO_16:
            CO_memcpy((uint8_t*)(bp->m_data.m_cellVols + 12), _data, 8);
            bp->m_data_count++;
            break;
        case BP_TEMP_TPDO_COBID:
            CO_memcpy(bp->m_data.m_temps, _data, 6);
            bp->m_data_count++;
            break;
        case BP_SOH_CYCLE_COBID:
            bp->m_data.m_soh = (uint8_t)_data[2];
            bp->m_data.m_cycle = CO_getUint16(_data);
            bp->m_data_count++;
            break;
        default:
            break;
    }
    elapsed_timer_reset(&bp->m_timeout);

    if(_this->m_update_fn && bp->m_data_count >= BP_DATA_FIELD_NUMBER){
        _this->m_update_fn(bp->m_id, &bp->m_data, _this->m_update_arg);

        bp->m_data_count = 0;
    }
}

static void sm_bp_co_received_data(uint32_t _can_id, uint8_t* _data, void* _arg){
    sm_bp_co_impl_t* bp_co = (sm_bp_co_impl_t*)(_arg);
    switch(_can_id & 0xFFFFFF80){
        case CO_CAN_ID_TPDO_1:
        case CO_CAN_ID_TPDO_2:
        case CO_CAN_ID_TPDO_3:
        case CO_CAN_ID_TPDO_4:
        case CO_CAN_ID_RPDO_1:
        case CO_CAN_ID_RPDO_2:
        case CO_CAN_ID_RPDO_3:
            sm_bp_co_extract_pdo(bp_co, _can_id, _data);
            break;
        default:
            break;
    }
}

static void sm_co_bp_sdo_cb(SM_SDO_STATUS_t _status, int32_t _tx_err, int32_t _rx_err, void* _arg){
    sm_bp_co_impl_t* bp_co = (sm_bp_co_impl_t*)(_arg);
    if(!bp_co || !bp_co->m_cmd){
        return;
    }

    if(_status == SM_SDO_ST_SUCCESS){
        LOG_ERR(TAG, "Bp %d command FAILURE: %d, TX_ERROR: 0x%02X, RX_ERROR: 0x%02X",
                bp_co->m_cmd->m_id,
                bp_co->m_cmd->m_cmd,
                _tx_err,
                _rx_err);

        if(bp_co->m_cmd->m_cb){
            bp_co->m_cmd->m_cb(bp_co->m_cmd->m_id,
                               bp_co->m_cmd->m_cmd,
                               SM_BP_CMD_SUCCESS,
                               bp_co->m_cmd->m_data,
                               bp_co->m_cmd->m_arg);
        }
    }else if(_status == SM_SDO_ST_ABORT){
        LOG_ERR(TAG, "Bp %d command FAILURE: %d, TX_ERROR: 0x%02X, RX_ERROR: 0x%02X",
                bp_co->m_cmd->m_id,
                bp_co->m_cmd->m_cmd,
                _tx_err,
                _rx_err);

        if(bp_co->m_cmd->m_cb){
            bp_co->m_cmd->m_cb(bp_co->m_cmd->m_id,
                               bp_co->m_cmd->m_cmd,
                               SM_BP_CMD_FAILURE,
                               bp_co->m_cmd->m_data,
                               bp_co->m_cmd->m_arg);
        }
    }

    sm_bp_co_reset_cmd(bp_co);
}

static int32_t sm_bp_co_reboot(sm_bp_co_t* _this){
    LOG_INF(TAG, "BP Reboot command");
    _impl(_this)->m_cmd_temp = 1;
    return sm_co_sdo_send(_impl(_this)->m_co,
                          SDO_BOOTLOADER_INDEX,
                          SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          &_impl(_this)->m_cmd_temp,
                          1,
                          100,
                          sm_co_bp_sdo_cb,
                          _this);

 /*   _impl(_this)->m_cmd_temp = 1;

    struct CO_Sub_Object_t subObject = {
            .p_data = & _impl(_this)->m_cmd_temp,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    if(CO_SDO_get_status(&CO_DEVICE.sdo_client) == CO_SDO_RT_busy){
        LOG_ERR(MAIN_TAG, "SDO Status BUSY");
        return -1;
    }

    CO_SDOclient_start_download(&_impl(_this)->m_co->sdo_client,
                              _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
							  SDO_BOOTLOADER_INDEX,
							  SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX,
                              &subObject,
                              100);

    return 0;*/
}

int32_t sm_bp_co_charge(sm_bp_co_t* _this){
    LOG_INF(TAG, "SDO CHARGE BMS_INDEX: %x, SUB_INDEX: %x, NODE_ID: %d", BMS_INDEX, BMS_MAIN_SWITCH_SUB_INDEX, _impl(_this)->m_cmd->m_id);
    _impl(_this)->m_cmd_temp = BMS_STATE_CHARGING;
    return sm_co_sdo_send(_impl(_this)->m_co,
                          BMS_INDEX,
                          BMS_MAIN_SWITCH_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          &_impl(_this)->m_cmd_temp,
                          1,
                          SDO_WRITE_OBJ_TIMEOUT_MS,
                          sm_co_bp_sdo_cb,
                          _this);

   /* struct CO_Sub_Object_t subObject = {
            .p_data = & _impl(_this)->m_cmd_temp,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    CO_SDOclient_start_download(&_impl(_this)->m_co->sdo_client,
                              _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                              BMS_INDEX,
                              BMS_MAIN_SWITCH_SUB_INDEX,
                              &subObject,
                              SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;*/
}

int32_t sm_bp_co_only_discharge(sm_bp_co_t* _this){
    _impl(_this)->m_cmd_temp = BMS_STATE_ONLY_DISCHARGING;
    return sm_co_sdo_send(_impl(_this)->m_co,
                          BMS_INDEX,
                          BMS_MAIN_SWITCH_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          &_impl(_this)->m_cmd_temp,
                          1,
                          SDO_WRITE_OBJ_TIMEOUT_MS,
                          sm_co_bp_sdo_cb,
                          _this);

/*    struct CO_Sub_Object_t subObject = {
            .p_data = & _impl(_this)->m_cmd_temp,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    CO_SDOclient_start_download(&_impl(_this)->m_co->sdo_client,
                                _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                                BMS_INDEX,
                                BMS_MAIN_SWITCH_SUB_INDEX,
                                &subObject,
                                SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;*/
}

int32_t sm_bp_co_discharge(sm_bp_co_t* _this){
    LOG_INF(TAG, "SDO DISCHARGE BMS_INDEX: %x, SUB_INDEX: %x, NODE_ID: %d", BMS_INDEX, BMS_MAIN_SWITCH_SUB_INDEX, _impl(_this)->m_cmd->m_id);
    _impl(_this)->m_cmd_temp = BMS_STATE_DISCHARGING;

    return sm_co_sdo_send(_impl(_this)->m_co,
                          BMS_INDEX,
                          BMS_MAIN_SWITCH_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          &_impl(_this)->m_cmd_temp,
                          1,
                          SDO_WRITE_OBJ_TIMEOUT_MS,
                          sm_co_bp_sdo_cb,
                          _this);

/*    struct CO_Sub_Object_t subObject = {
            .p_data = & _impl(_this)->m_cmd_temp,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    CO_SDOclient_start_download(&_impl(_this)->m_co->sdo_client,
                              _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                              BMS_INDEX,
                              BMS_MAIN_SWITCH_SUB_INDEX,
                              &subObject,
                              SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;*/
}

int32_t sm_bp_co_standby(sm_bp_co_t *_this){
    LOG_INF(TAG, "SDO STANDBY BMS_INDEX: %x, SUB_INDEX: %x, NODE_ID: %d", BMS_INDEX, BMS_MAIN_SWITCH_SUB_INDEX, _impl(_this)->m_cmd->m_id);

    _impl(_this)->m_cmd_temp = BMS_STATE_STANDBY;
    return sm_co_sdo_send(_impl(_this)->m_co,
                          BMS_INDEX,
                          BMS_MAIN_SWITCH_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          &_impl(_this)->m_cmd_temp,
                          1,
                          SDO_WRITE_OBJ_TIMEOUT_MS,
                          sm_co_bp_sdo_cb,
                          _this);

    /* _impl(_this)->m_cmd_temp = BMS_STATE_STANDBY;

      struct CO_Sub_Object_t subObject = {
               .p_data = & _impl(_this)->m_cmd_temp,
               .attr = ODA_SDO_RW,
               .len = 1,
               .p_ext = NULL,
               .p_temp_data = NULL
       };

       CO_SDOclient_start_download(&_impl(_this)->m_co->sdo_client,
                                   _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                                   BMS_INDEX,
                                   BMS_MAIN_SWITCH_SUB_INDEX,
                                   &subObject,
                                   SDO_WRITE_OBJ_TIMEOUT_MS);


       return 0;*/
}
int32_t sm_bp_co_read_sn(sm_bp_co_t* _this){
    LOG_INF(TAG, "SDO READ SN BMS_INDEX: %x, SUB_INDEX: %x, NODE_ID: %d", BMS_INDEX, BMS_SERIAL_NUMBER_OBJECT_SUB_INDEX, _impl(_this)->m_cmd->m_id);
    return sm_co_sdo_receive(_impl(_this)->m_co,
                          BMS_INDEX,
                          BMS_SERIAL_NUMBER_OBJECT_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          _impl(_this)->m_cmd->m_data,
                          BP_DEVICE_SN_SIZE,
                          SDO_READ_OBJ_TIMEOUT_MS,
                          sm_co_bp_sdo_cb,
                          _this);

/*    struct CO_Sub_Object_t subObject = {
            .p_data =  _impl(_this)->m_cmd->m_data,
            .attr = ODA_SDO_RW,
            .len = BP_DEVICE_SN_SIZE,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    CO_SDOclient_start_upload(&_impl(_this)->m_co->sdo_client,
                              _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                              BMS_INDEX,
                              BMS_SERIAL_NUMBER_OBJECT_SUB_INDEX,
                              &subObject,
                              SDO_READ_OBJ_TIMEOUT_MS);

    return 0;*/
}

int32_t sm_bp_co_read_version(sm_bp_co_t* _this){
    return sm_co_sdo_send(_impl(_this)->m_co,
                          BMS_VERSION_INDEX,
                          BMS_VERSION_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          _impl(_this)->m_cmd->m_data,
                          BP_VERSION_SIZE,
                          SDO_READ_OBJ_TIMEOUT_MS,
                          sm_co_bp_sdo_cb,
                          _this);

  /*  struct CO_Sub_Object_t subObject = {
            .p_data = _impl(_this)->m_cmd->m_data,
            .attr = ODA_SDO_RW,
            .len = BP_VERSION_SIZE,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    CO_SDOclient_start_upload(&_impl(_this)->m_co->sdo_client,
                              _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
							  BMS_VERSION_INDEX,
                              BMS_VERSION_SUB_INDEX,
                              &subObject,
                              SDO_READ_OBJ_TIMEOUT_MS);

    return 0;*/
}

int32_t sm_bp_co_read_dev(sm_bp_co_t* _this){
    return sm_co_sdo_receive(_impl(_this)->m_co,
                          BMS_VEHICLE_SN_INDEX,
                          BMS_MATTED_DEV_SUB_INDEX,
                          _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                          _impl(_this)->m_cmd->m_data,
                          BP_DEVICE_SN_SIZE,
                          SDO_READ_OBJ_TIMEOUT_MS,
                          sm_co_bp_sdo_cb,
                          _this);
/*    struct CO_Sub_Object_t subObject = {
            .p_data =  _impl(_this)->m_cmd->m_data,
            .attr = ODA_SDO_RW,
            .len = BP_DEVICE_SN_SIZE,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    CO_SDOclient_start_upload(&_impl(_this)->m_co->sdo_client,
                              _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                              BMS_VEHICLE_SN_INDEX,
                              BMS_MATTED_DEV_SUB_INDEX,
                              &subObject,
                              SDO_READ_OBJ_TIMEOUT_MS);

    return 0;*/
}

int32_t sm_bp_co_write_dev(sm_bp_co_t* _this){
    return sm_co_sdo_send(_impl(_this)->m_co,
                   BMS_VEHICLE_SN_INDEX,
                   BMS_MATTED_DEV_SUB_INDEX,
                   _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                   _impl(_this)->m_cmd->m_data,
                   BP_DEVICE_SN_SIZE,
                   SDO_WRITE_OBJ_TIMEOUT_MS,
                   sm_co_bp_sdo_cb,
                   _this);

/*    struct CO_Sub_Object_t subObject = {
            .p_data =  _impl(_this)->m_cmd->m_data,
            .attr = ODA_SDO_RW,
            .len = BP_DEVICE_SN_SIZE,
            .p_ext = NULL,
            .p_temp_data = NULL
    };

    CO_SDOclient_start_download(&_impl(_this)->m_co->sdo_client,
                              _impl(_this)->m_cmd->m_id + BP_NODE_ID_OFFSET,
                              BMS_VEHICLE_SN_INDEX,
                              BMS_MATTED_DEV_SUB_INDEX,
                              &subObject,
                              SDO_WRITE_OBJ_TIMEOUT_MS);

    return 0;*/
}

sm_bp_co_t* sm_bp_co_create(sm_co_t* _co,
                            sm_bp_t* _bp_list,
                            uint8_t _bp_num, 
                            sm_bp_co_update_data_fn_t _update_data_fn, 
                            void* _arg){
    if(!_co || !_bp_list || !_bp_num){
        return NULL;
    }               
    g_bp_co.m_bp_numb = _bp_num;
    g_bp_co.m_bps = _bp_list;

    g_bp_co.m_update_arg = _arg;
    g_bp_co.m_update_fn = _update_data_fn;

    g_bp_co.m_sync = 1;

    g_bp_co.m_cmd = NULL;
    g_bp_co.m_cmd_temp = 0;

    g_bp_co.m_co_if = sm_co_get_co_if(_co);
    g_bp_co.m_co = _co;

    sm_co_if_reg_recv_callback(g_bp_co.m_co_if, sm_bp_co_received_data, &g_bp_co);

    return &g_bp_co;
}

int32_t sm_bp_co_destroy(sm_bp_co_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_bps = NULL;
    _impl(_this)->m_update_fn = NULL;
    _impl(_this)->m_update_arg = NULL;
    
    return 0;
}

int32_t sm_bp_co_reset_cmd(sm_bp_co_t* _this){
    _impl(_this)->m_cmd = NULL;
//    sm_co_sdo_reset(_impl(_this)->m_co);
    return 0;
}

int32_t sm_bp_co_is_busy(sm_bp_co_t* _this){
    if(!_this || !_impl(_this)->m_co || !_impl(_this)->m_sync){
        return 0;
    }
    return sm_co_sdo_is_busy(_impl(_this)->m_co);
}

int32_t sm_bp_co_set_cmd_force(sm_bp_co_t* _this, const sm_bp_cmd_t* _cmd){
    if(_impl(_this)->m_cmd && _impl(_this)->m_cmd->m_cb){
        const sm_bp_cmd_t* cmd = _impl(_this)->m_cmd;
        cmd->m_cb(cmd->m_id, cmd->m_cmd, SM_BP_CMD_FAILURE, cmd->m_data, cmd->m_arg);
    }
    sm_co_sdo_reset(_impl(_this)->m_co);
    return sm_bp_co_set_cmd(_this, _cmd);
}

int32_t sm_bp_co_set_cmd(sm_bp_co_t* _this, const sm_bp_cmd_t* _cmd){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cmd = _cmd;

    switch (_cmd->m_cmd){
        case BP_CMD_REBOOT:
            sm_bp_co_reboot(_this);
            break;

        case BP_CMD_CHARGE:
            sm_bp_co_charge(_this);
            break;

        case BP_CMD_ONLY_DISCHARGE:
            sm_bp_co_only_discharge(_this);
            break;

        case BP_CMD_DISCHARGE:
            sm_bp_co_discharge(_this);
            break;
        case BP_CMD_STANDBY:
            sm_bp_co_standby(_this);
            break;
        case BP_CMD_READ_SN:
            sm_bp_co_read_sn(_this);
            break;

        case BP_CMD_READ_VERSION:
            sm_bp_co_read_version(_this);
            break;

        case BP_CMD_READ_ASSIGNED_DEV:
            sm_bp_co_read_dev(_this);
            break;

        case BP_CMD_WRITE_ASSIGNED_DEV:
            sm_bp_co_write_dev(_this);
            break;

        default:
        	return -1;
    }
    return 0;
}

int32_t sm_bp_co_process(sm_bp_co_t* _this){
    return 0;
}
