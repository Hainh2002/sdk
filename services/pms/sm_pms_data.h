//
// Created by DELL 5425 on 4/11/2024.
//
#include "sm_bp_data.h"
#include "stdio.h"
#ifndef EV_SDK_SM_PMS_DATA_H
#define EV_SDK_SM_PMS_DATA_H

#define SM_SV_PMS_MAX_BP_NUM_DEFAULT							3
#define SM_SV_PMS_VOL_DIFF_SWITCH_DISCHARGE_DEFAULT 			2000	//mV
#define SM_SV_PMS_VOL_DIFF_MERGE_DISCHARGE_DEFAULT				2000		//mV
#define SM_SV_PMS_MAX_CUR_MERGE_DISCHARGE_DEFAULT	 			18000	//mA
#define SM_SV_PMS_VOL_DIFF_SWITCH_CHARGE_DEFAULT 				300	    //mV
#define SM_SV_PMS_VOL_DIFF_MERGE_CHARGE_DEFAULT					300		//mV
#define SM_SV_PMS_MAX_CUR_MERGE_CHARGE_DEFAULT	 				18000	//mA
#define SM_SV_PMS_SWITCH_TIMEOUT								5000	//ms
#define SM_SV_PMS_MERGE_TIMEOUT									5000	//ms

typedef enum {
    SM_PMS_CMD_IMPL_PORT_STANDBY,
    SM_PMS_CMD_IMPL_PORT_DISCHARGE,
    SM_PMS_CMD_IMPL_PORT_ONLY_DISCHARGE,
    SM_PMS_CMD_NEXT_PORT_DISCHARGE,
    SM_PMS_CMD_NULL,
}SM_PMS_CMD;

typedef enum {
    PORT_ST_DISABLE,
    PORT_ST_ENABLE,
    PORT_ST_FORCED_SWITCH,
    PORT_ST_FORCED_MERGE,
    PORT_ST_NO_CHECK,
} PORT_ST;

typedef struct {
    sm_bp_data_t            *m_bp_data;
    uint8_t                 m_bp_state;
    uint8_t                 m_valid_st;
    uint8_t                 m_action_st;
    uint8_t                 m_connection_st;
    uint8_t                 m_switch_merge_st;
} pms_port_t;

/**
 * data structure used for data transmitting to app and other services.
 */
typedef struct pms_data {
    int8_t                  m_sorted_port[SM_SV_PMS_MAX_BP_NUM_DEFAULT];
    int8_t                  m_impl_port_id;
    int8_t                  m_switch_merge_st;
    uint8_t                 m_activity_st;          // Charging or Discharging
} sm_pms_data_t;

typedef struct pms_config {
    uint32_t m_vol_diff_switch_discharge;
    uint32_t m_vol_diff_merge_discharge;
    uint32_t m_vol_diff_switch_charge;
    uint32_t m_vol_diff_merge_charge;
    uint32_t m_max_cur_merge_discharge;
    uint32_t m_max_cur_merge_charge;
    uint32_t m_switch_timeout;
    uint32_t m_merge_timeout;
    uint8_t	 m_max_bp_num;
}sm_sv_pms_config_t;

static inline void pms_data_reset(sm_pms_data_t *_data){
    for (int i = 0; i < SM_SV_PMS_MAX_BP_NUM_DEFAULT; ++i) {
        _data->m_sorted_port[i] = -1;
    }
    _data->m_impl_port_id = -1;
    _data->m_activity_st = 0;
    _data->m_switch_merge_st = 0;
}
static inline void pms_clone_data(sm_pms_data_t *_des, const sm_pms_data_t* _src){
    if (!_src || !_des)
        return;
    for (int i = 0; i < SM_SV_PMS_MAX_BP_NUM_DEFAULT; ++i) {
        _des->m_sorted_port[i] = _src->m_sorted_port[i];
    }
    _des->m_impl_port_id    = _src->m_impl_port_id;
    _des->m_activity_st     = _src->m_activity_st;
    _des->m_switch_merge_st = _src->m_switch_merge_st;
}
static inline void pms_log_data(sm_pms_data_t *_data){
    printf("Sorted port: %d %d %d\n",_data->m_sorted_port[0], _data->m_sorted_port[1], _data->m_sorted_port[2]);
    printf("Impl-port: %d - Port-id: %d\n", _data->m_impl_port_id, _data->m_sorted_port[_data->m_impl_port_id]);
    printf("Mode: %d\n", _data->m_activity_st);
}
#endif //EV_SDK_SM_PMS_DATA_H
