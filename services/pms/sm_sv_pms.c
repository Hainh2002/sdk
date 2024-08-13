//
// Created by DELL 5425 on 3/22/2024.
//
#include <stdlib.h>
#include "sm_sv_pms.h"
#include "sm_bp_data.h"
#include "sm_logger.h"
#include "sm_pms_est_data.h"
#include "sm_elapsed_timer.h"

#define SM_SV_PMS_ESTIMATE_ENABLE   (1)

#define SM_BP_CMD_NULL (1)

static const char* TAG = "SM_SV_PMS";

typedef enum {
    SM_SV_PMS_ST_NO_SWITCH,
    SM_SV_PMS_ST_PRE_SWITCH,
    SM_SV_PMS_ST_MAIN_SWITCH,
    SM_SV_PMS_ST_FINISH_SWITCH,
    SM_SV_PMS_ST_MERGE,
    SM_SV_PMS_ST_RECOVERY,
    SM_SV_PMS_ST_FORCE_SWITCH,
    SM_SV_PMS_ST_FORCE_MERGE,
    SM_SV_PMS_ST_SWITCH_FAIL,
} SM_SV_PMS_OPERATION_ST;

enum {
    PMS_ST_INIT,
    PMS_ST_DISCHARGING,
    PMS_ST_CHARGING
} PMS_ST;

enum {
    PORT_TRASH = -1,
    PORT_0,
    PORT_1,
    PORT_2,
    PORT_3,
    PORT_4,
    PORT_5,
    PORT_6,
    PORT_7
};

typedef struct {
    sm_sv_bp_t              *m_bp;
    sm_sv_pms_config_t      *m_config;
    pms_port_t              *m_port;
    int8_t                  m_sorted_port[SM_SV_PMS_MAX_BP_NUM_DEFAULT];
    int8_t                  m_impl_port_id;
    int8_t                  m_next_port_id;
    uint8_t                 m_pms_state;
    uint8_t                 m_switch_merge_st;
    elapsed_timer_t         m_switch_merge_timeout;
    elapsed_timer_t         m_wait_recovery_timeout;

    int32_t                 m_sdo_st;
    int32_t                 m_sdo_cmd;
    int32_t                 m_port_in_cmd;

    est_data_t              *m_est;
    sm_pms_data_t           *m_pms_data_tx;
    sm_pms_data_t           *m_pms_data_rx;
    uint8_t                 m_is_new_data;

    sm_sv_pms_event_cb_fn_t *event_cb_fn_t;
    void                    *m_event_arg;
} sm_sv_pms;

#define impl(x) ((sm_sv_pms*)(x))

est_time_cfg_t g_est_time = {
        .m_dist_duration        = PMS_EST_DISTANCE_DURATION,
        .m_cur_duration         = PMS_EST_CUR_MAX_DURATION,
        .m_power_duration       = PMS_EST_POWER_KM_DURATION,
};
est_data_t g_est_data_default = {
        .m_distance_km = 0,
        .m_distance_m = 0,
        .m_cur_max = 0,
        .m_power_per_km = 0,
        .m_duration = &g_est_time
};

sm_sv_pms_config_t g_pms_cfg_default = {
        .m_max_bp_num                 = SM_SV_PMS_MAX_BP_NUM_DEFAULT,
        .m_max_cur_merge_charge       = SM_SV_PMS_MAX_CUR_MERGE_CHARGE_DEFAULT,
        .m_max_cur_merge_discharge    = SM_SV_PMS_MAX_CUR_MERGE_DISCHARGE_DEFAULT,
        .m_vol_diff_switch_charge     = SM_SV_PMS_VOL_DIFF_SWITCH_CHARGE_DEFAULT,
        .m_vol_diff_switch_discharge  = SM_SV_PMS_VOL_DIFF_SWITCH_DISCHARGE_DEFAULT,
        .m_vol_diff_merge_charge      = SM_SV_PMS_VOL_DIFF_MERGE_CHARGE_DEFAULT,
        .m_vol_diff_merge_discharge   = SM_SV_PMS_VOL_DIFF_MERGE_DISCHARGE_DEFAULT,
        .m_switch_timeout             = SM_SV_PMS_SWITCH_TIMEOUT,
        .m_merge_timeout              = SM_SV_PMS_MERGE_TIMEOUT,
};

static sm_bp_data_t g_pms_bp_data_handle[SM_SV_PMS_MAX_BP_NUM_DEFAULT];
static pms_port_t g_pms_port_handle[SM_SV_PMS_MAX_BP_NUM_DEFAULT];
static sm_pms_data_t g_pms_data_tx_buff;
static sm_pms_data_t g_pms_data_rx_buff;

static int8_t is_port_in_sorted_list(sm_sv_pms_t *, int8_t);
static void pms_reset_sdo_status(sm_sv_pms_t* _this);
static void pms_set_bp_cmd(sm_sv_pms_t *_this, SM_BP_CMD _cmd, uint8_t _id);

sm_sv_pms_t* sm_sv_pms_create(sm_sv_bp_t* _bp){
    sm_sv_pms *_this = malloc(sizeof(sm_sv_pms));
    if (_this == NULL){
        return NULL;
    }

    _this->m_bp                 = _bp;
    _this->m_config             = &g_pms_cfg_default;
    _this->m_pms_state          = PMS_ST_INIT;
    _this->m_port               = g_pms_port_handle;
    _this->m_switch_merge_st    = SM_SV_PMS_ST_NO_SWITCH;
    elapsed_timer_resetz(&_this->m_switch_merge_timeout, SM_SV_PMS_SWITCH_TIMEOUT);

    _this->m_est                = &g_est_data_default;
    sm_pms_reset_data(_this->m_est);
    _this->m_pms_data_tx        = &g_pms_data_tx_buff;
    _this->m_pms_data_rx        = &g_pms_data_rx_buff;
    pms_data_reset(_this->m_pms_data_tx);
    pms_data_reset(_this->m_pms_data_rx);

    _this->m_is_new_data        = 0;

    for (int i = 0; i < _this->m_config->m_max_bp_num; ++i) {
        _this->m_port[i].m_bp_data                  = &g_pms_bp_data_handle[i];
        _this->m_port[i].m_action_st                = PORT_ST_DISABLE;
        _this->m_port[i].m_connection_st            = PORT_ST_DISABLE;
        _this->m_port[i].m_switch_merge_st          = PORT_ST_ENABLE;
        _this->m_port[i].m_valid_st                 = PORT_ST_NO_CHECK;
        _this->m_port[i].m_bp_state                 = BP_STATE_IDLE;
        _this->m_sorted_port[i]                     = PORT_TRASH;
    }
    _this->m_impl_port_id               = PORT_TRASH;
    _this->m_next_port_id               = PORT_TRASH;



    return (sm_sv_pms_t*)_this;
}

int32_t sm_sv_pms_destroy(sm_sv_pms_t* _this){
    if (_this == NULL)
        return -1;
    impl(_this)->m_bp = NULL;
    impl(_this)->m_config =  NULL;
    impl(_this)->event_cb_fn_t = NULL;
    impl(_this)->m_event_arg = NULL;
    free(_this);
    _this = NULL;
    return 0;
}

int32_t sm_sv_pms_set_config(sm_sv_pms_t* _this, sm_sv_pms_config_t* _pms_config){
    if (_this == NULL) return -1;
    impl(_this)->m_config = _pms_config;
    return 0;
}

int32_t sm_sv_pms_get_config(sm_sv_pms_t* _this, sm_sv_pms_config_t* _pms_config){
    if (_this == NULL) return -1;
    _pms_config = impl(_this)->m_config;
    return 0;
}

int32_t sm_sv_pms_reg_event(sm_sv_pms_t* _this,
                            sm_sv_pms_event_cb_fn_t *_cb,
                            void* _arg){
    if (_this == NULL)
        return -1;
    impl(_this)->event_cb_fn_t = _cb;
    impl(_this)->m_event_arg = _arg;
    return 0;
}

int32_t sm_sv_pms_switch_merge_enable(sm_sv_pms_t* _this, uint8_t _port, uint8_t _is_enable){
    if (_this == NULL) return -1;
    if (_port >= impl(_this)->m_config->m_max_bp_num || _port < 0)
        return -1;
    if (_is_enable){
        impl(_this)->m_port[_port].m_switch_merge_st = PORT_ST_ENABLE;
        return _port;
    }else{
        impl(_this)->m_port[_port].m_switch_merge_st = PORT_ST_DISABLE;
        return _port;
    }
}
int32_t sm_sv_pms_get_active_port(sm_sv_pms_t* _this, int8_t* _active_port[]){
    if (_this == NULL) return -1;

    uint8_t size = 0;
    static int8_t port[SM_SV_PMS_MAX_BP_NUM_DEFAULT] = {0};
    memset(port, -1, SM_SV_PMS_MAX_BP_NUM_DEFAULT);

    for (int8_t i=0; i< impl(_this)->m_config->m_max_bp_num; ++i){
        if (impl(_this)->m_port[i].m_connection_st != PORT_ST_ENABLE ||    // BP disconnect
            impl(_this)->m_port[i].m_action_st != PORT_ST_ENABLE ||        // BP standby
            impl(_this)->m_port[i].m_valid_st == PORT_ST_DISABLE)          // BP invalid
            continue;
        port[size] = i;
        *_active_port[size] = port[size];
        size++;
    }
    return size;
}
int32_t sm_sv_pms_get_inactive_port(sm_sv_pms_t* _this, int8_t* _inactive_port[]){
    if (_this == NULL) return -1;

    uint8_t size = 0;
    static int8_t port[SM_SV_PMS_MAX_BP_NUM_DEFAULT];
    memset(port, -1, SM_SV_PMS_MAX_BP_NUM_DEFAULT);

    for (int8_t i=0; i< impl(_this)->m_config->m_max_bp_num; ++i){
        if (impl(_this)->m_port[i].m_connection_st != PORT_ST_ENABLE ||    // BP disconnect
            impl(_this)->m_port[i].m_action_st != PORT_ST_ENABLE ||        // BP standby
            impl(_this)->m_port[i].m_valid_st == PORT_ST_DISABLE)          // BP invalid
        {
            port[size] = i;
            *_inactive_port[size] = port[size];
            size++;
        }
    }
    return size;
}

int32_t sm_sv_pms_force_switch(sm_sv_pms_t* _this, int8_t _forced_bp_id){
    if (_this == NULL ||
        _forced_bp_id == PORT_TRASH ||
        impl(_this)->m_port[_forced_bp_id].m_connection_st == PORT_ST_DISABLE ||
        impl(_this)->m_port[_forced_bp_id].m_switch_merge_st == PORT_ST_DISABLE ||
        impl(_this)->m_port[_forced_bp_id].m_switch_merge_st == PORT_ST_FORCED_SWITCH||
        impl(_this)->m_switch_merge_st == SM_SV_PMS_ST_FORCE_SWITCH ||
        impl(_this)->m_sdo_cmd != BP_CMD_NUMBER)
        return -1;

    if (impl(_this)->m_port[_forced_bp_id].m_bp_state == BP_STATE_DISCHARGING){
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_FORCE_SWITCH;
        impl(_this)->m_impl_port_id =  is_port_in_sorted_list(_this, _forced_bp_id);
    }
    else {
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_PRE_SWITCH;
        impl(_this)->m_next_port_id = is_port_in_sorted_list(_this, _forced_bp_id);
    }

    impl(_this)->m_port[_forced_bp_id].m_switch_merge_st = PORT_ST_FORCED_SWITCH;
    elapsed_timer_reset(&impl(_this)->m_switch_merge_timeout);
    return 0;
}
int32_t sm_sv_pms_release_switch(sm_sv_pms_t* _this, int8_t _released_bp_id){
    if (_this == NULL) return -1;
    impl(_this)->m_port[_released_bp_id].m_switch_merge_st = PORT_ST_ENABLE;
    if (is_port_in_sorted_list(_this, _released_bp_id) == PORT_TRASH)
        for (uint8_t i=0; i< impl(_this)->m_config->m_max_bp_num; ++i){
            if (impl(_this)->m_sorted_port[i] != PORT_TRASH) continue;
            impl(_this)->m_sorted_port[i] = (int8_t) _released_bp_id;
        }
    if (impl(_this)->m_switch_merge_st == SM_SV_PMS_ST_FORCE_SWITCH)
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
    return 0;
}
/*int32_t sm_sv_pms_force_merge(sm_sv_pms_t* _this, int8_t _forced_bp_id){
    if (_this == NULL ||
        _forced_bp_id == PORT_TRASH ||
        impl(_this)->m_port[_forced_bp_id].m_connection_st == PORT_ST_DISABLE ||
        impl(_this)->m_port[_forced_bp_id].m_switch_merge_st == PORT_ST_DISABLE ||
        impl(_this)->m_sdo_cmd != BP_CMD_NUMBER)
        return -1;
    impl(_this)->m_pms_state = SM_SV_PMS_ST_MERGE;
    impl(_this)->m_port[_forced_bp_id].m_switch_merge_st = PORT_ST_FORCED_MERGE;
    impl(_this)->m_next_port_id = is_port_in_sorted_list(_this, _forced_bp_id);
}*/

static void pms_cmd_cb_handle(int32_t _id, SM_BP_CMD _cmd, int32_t _sdo_st, void* _data, void* _arg){
    impl(_arg)->m_sdo_st = _sdo_st;
    impl(_arg)->m_sdo_cmd = _cmd;
    impl(_arg)->m_port_in_cmd = _id;
    LOG_DBG(TAG, "SDO CALLBACK : cmd %d st %d port %d", _cmd, _sdo_st, _id);
    if (_sdo_st == SM_BP_CMD_SUCCESS){
        switch (_cmd) {
            case BP_CMD_DISCHARGE:
                impl(_arg)->m_port[_id].m_bp_state = BP_STATE_DISCHARGING;
                break;
            case BP_CMD_ONLY_DISCHARGE:
                impl(_arg)->m_port[_id].m_bp_state = BP_STATE_ONLY_DISCHARGING;
                break;
            case BP_CMD_STANDBY:
                impl(_arg)->m_port[_id].m_bp_state = BP_STATE_STANDBY;
                break;
            case BP_CMD_REBOOT:
//                impl(_arg)->m_port[_id].m_bp_state = BP_STATE_DISCHARGING;
                break;
        }
    }
    sm_sv_bp_reset_current_cmd(impl(_arg)->m_bp);
}
static void pms_reset_sdo_status(sm_sv_pms_t* _this){
    impl(_this)->m_sdo_st = SM_BP_CMD_NULL;
    impl(_this)->m_sdo_cmd = BP_CMD_NUMBER;
    impl(_this)->m_port_in_cmd = PORT_TRASH;
}
static void pms_update_switch_discharge_st(sm_sv_pms_t* _this){
    int8_t impl_port = impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id];
    int8_t next_port = impl(_this)->m_sorted_port[impl(_this)->m_next_port_id];

    if (impl_port == PORT_TRASH ||
        next_port == impl_port ||
        next_port == PORT_TRASH) return;

    int32_t vol_dif = impl(_this)->m_port[next_port].m_bp_data->m_vol*10 - impl(_this)->m_port[impl_port].m_bp_data->m_vol*10;
    if (abs(vol_dif) <= impl(_this)->m_config->m_vol_diff_merge_discharge ){
        // next = impl
        pms_reset_sdo_status(_this);
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_MERGE;
        return;
    }

    if (abs(vol_dif) > impl(_this)->m_config->m_vol_diff_switch_discharge){
        if (vol_dif > 0) // next > impl
        {
            impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_PRE_SWITCH;
        }
        else
        {
            impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        }
    }
}
static void pms_sort_port(sm_sv_pms_t* _this){
    for (int i = 0; i < impl(_this)->m_config->m_max_bp_num; ++i) {
        if (impl(_this)->m_sorted_port[i] == PORT_TRASH) continue;
        for (int j = i+1; j < impl(_this)->m_config->m_max_bp_num; ++j) {
            if (impl(_this)->m_sorted_port[j] == PORT_TRASH) continue;
            if (impl(_this)->m_port[impl(_this)->m_sorted_port[j]].m_bp_data->m_soc
                > impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_data->m_soc)
            {   // swap order port_id
                LOG_DBG(TAG,"Swap slot %d and %d",i,j);
                int8_t tmp = impl(_this)->m_sorted_port[i];
                impl(_this)->m_sorted_port[i] = impl(_this)->m_sorted_port[j];
                impl(_this)->m_sorted_port[j] = tmp;
                // move impl_port_id
                if (impl(_this)->m_impl_port_id == i)
                    impl(_this)->m_impl_port_id = j;
                else if (impl(_this)->m_impl_port_id == j)
                    impl(_this)->m_impl_port_id = i;
                LOG_DBG(TAG,"Impl port: %d",impl(_this)->m_impl_port_id);
            }
        }
    }
    for (int8_t i=0; i<impl(_this)->m_config->m_max_bp_num; ++i)
    {
//        LOG_DBG(TAG, "Sorted_port [%d] index %d ",i, impl(_this)->m_sorted_port[i]);
    }
}
static void pms_pre_switch_handle(sm_sv_pms_t* _this){ // set discharge_port into only discharge
    // Check SDO
    if (impl(_this)->m_sdo_st == SM_BP_CMD_SUCCESS && impl(_this)->m_sdo_cmd == BP_CMD_ONLY_DISCHARGE)
    {
        for (int i = 0; i< impl(_this)->m_config->m_max_bp_num;++i){
            if (impl(_this)->m_sorted_port[i] == PORT_TRASH) continue;
            if (impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_data->m_state == BP_STATE_DISCHARGING &&
                impl(_this)->m_sorted_port[i] == impl(_this)->m_port_in_cmd &&
                impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_state == BP_STATE_ONLY_DISCHARGING)
            {
                LOG_INF(TAG, "BP_CMD_ONLY_DISCHARGE success- port %d", impl(_this)->m_port_in_cmd);
                pms_reset_sdo_status(_this);
            }
        }

        if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER){
            for (int i = 0; i< impl(_this)->m_config->m_max_bp_num;++i) {
                if (impl(_this)->m_sorted_port[i] == PORT_TRASH) continue;
                if (impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_state == BP_STATE_DISCHARGING &&
                    impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_data->m_state == BP_STATE_DISCHARGING)
                {
                    pms_set_bp_cmd(_this, BP_CMD_ONLY_DISCHARGE, (int32_t)impl(_this)->m_sorted_port[i]);
                    LOG_INF(TAG, "BP_CMD_ONLY_DISCHARGE - port %d", impl(_this)->m_port_in_cmd);
                    elapsed_timer_reset(&impl(_this)->m_switch_merge_timeout);
                    return;
                }
            }
        }
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_MAIN_SWITCH;
        pms_reset_sdo_status(_this);
        return;
    }else if(impl(_this)->m_sdo_st == SM_BP_CMD_FAILURE &&
             impl(_this)->m_sdo_cmd == BP_CMD_ONLY_DISCHARGE){
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        LOG_ERR(TAG, "SET BP_CMD_ONLY_DISCHARGE FAIL !!!");
        pms_reset_sdo_status(_this);
        return;
    }

    if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER){
        pms_set_bp_cmd(_this,
                       BP_CMD_ONLY_DISCHARGE,
                       impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id]);
        LOG_INF(TAG, "BP_CMD_ONLY_DISCHARGE - port %d", impl(_this)->m_port_in_cmd);
        return;
        /*for (int i = 0; i< impl(_this)->m_config->m_max_bp_num;++i){
            if (impl(_this)->m_sorted_port[i] == PORT_TRASH) continue;
            if (impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_data->m_state == BP_STATE_DISCHARGING)
            {
                impl(_this)->m_port_in_cmd = (int32_t)impl(_this)->m_sorted_port[i];
                impl(_this)->m_sdo_cmd = BP_CMD_ONLY_DISCHARGE;
                impl(_this)->m_sdo_st = SM_BP_CMD_NULL;

                sm_sv_bp_set_cmd(impl(_this)->m_bp,
                                 impl(_this)->m_port_in_cmd,
                                 BP_CMD_ONLY_DISCHARGE,
                                 NULL,
                                 pms_cmd_cb_handle,
                                 _this);

                LOG_INF(TAG, "BP_CMD_ONLY_DISCHARGE - port %d", impl(_this)->m_port_in_cmd);
                return;
            }
        }*/
    }

}
static void pms_main_switch_handle(sm_sv_pms_t* _this){ // set next_port discharge
    // Check SDO
    if (impl(_this)->m_sdo_st == SM_BP_CMD_SUCCESS &&
        impl(_this)->m_sorted_port[impl(_this)->m_next_port_id] == impl(_this)->m_port_in_cmd)
    {
        if (impl(_this)->m_sdo_cmd == BP_CMD_DISCHARGE)
        {
            impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_FINISH_SWITCH;
            LOG_INF(TAG, "BP_CMD_DISCHARGE success - port %d", impl(_this)->m_port_in_cmd);
            pms_reset_sdo_status(_this);
            return;
        }
    }
    else if(impl(_this)->m_sdo_st == SM_BP_CMD_FAILURE &&
             impl(_this)->m_sdo_cmd == BP_CMD_DISCHARGE)
    {
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        pms_reset_sdo_status(_this);
        return;
    }

    if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER &&
        impl(_this)->m_port[impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]].m_bp_data->m_state != BP_STATE_DISCHARGING) {
        if (impl(_this)->m_pms_state == PMS_ST_DISCHARGING) {
            sm_sv_bp_set_cmd(impl(_this)->m_bp,
                             impl(_this)->m_sorted_port[impl(_this)->m_next_port_id],
                             BP_CMD_DISCHARGE,
                             NULL,
                             pms_cmd_cb_handle,
                             _this);

            impl(_this)->m_port_in_cmd = (int32_t) impl(_this)->m_sorted_port[impl(_this)->m_next_port_id];
            impl(_this)->m_sdo_cmd = BP_CMD_DISCHARGE;
            impl(_this)->m_sdo_st = SM_BP_CMD_NULL;
            LOG_INF(TAG, "BP_CMD_DISCHARGE - port %d", impl(_this)->m_port_in_cmd);
            elapsed_timer_reset(&impl(_this)->m_switch_merge_timeout);
        }
    }
}
static void pms_finish_switch_handle(sm_sv_pms_t* _this) { // set discharging port into standby
    // Check SDO
    if (impl(_this)->m_sdo_st == SM_BP_CMD_SUCCESS && impl(_this)->m_sdo_cmd == BP_CMD_STANDBY)
    {
        for (int i = 0; i< impl(_this)->m_config->m_max_bp_num;++i){
            if (impl(_this)->m_sorted_port[i] == PORT_TRASH) continue;
            if (impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_state == BP_STATE_STANDBY &&
                impl(_this)->m_sorted_port[i] == impl(_this)->m_port_in_cmd )
            {
                LOG_INF(TAG,"BP_CMD_STANDBY SUCCESS - PORT %d",impl(_this)->m_sorted_port[i]);
                pms_reset_sdo_status(_this);
            }
        }

        if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER){
            for (int i = 0; i< impl(_this)->m_config->m_max_bp_num;++i) {
                if (impl(_this)->m_sorted_port[i] == PORT_TRASH) continue;
                if (impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_state == BP_STATE_ONLY_DISCHARGING)
                {
                    pms_set_bp_cmd(_this, BP_CMD_STANDBY, (int32_t)impl(_this)->m_sorted_port[i]);
                    elapsed_timer_reset(&impl(_this)->m_switch_merge_timeout);
                    return;
                }
            }
        }
        impl(_this)->event_cb_fn_t->switch_merge_cb_fn(impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                                                       impl(_this)->m_sorted_port[ impl(_this)->m_next_port_id],
                                                       SM_SV_PMS_EVENT_SWITCH_SUCCESS,
                                                       _this);

        if (impl(_this)->m_port[impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]].m_switch_merge_st != PORT_ST_FORCED_SWITCH)
        {
            impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        }
        else {
            impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_FORCE_SWITCH;
        }

        impl(_this)->m_impl_port_id = impl(_this)->m_next_port_id;
        pms_reset_sdo_status(_this);
        return;
    }else if(impl(_this)->m_sdo_st == SM_BP_CMD_FAILURE &&
             impl(_this)->m_sdo_cmd == BP_CMD_STANDBY)
    {
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_RECOVERY;
        pms_reset_sdo_status(_this);
        impl(_this)->m_port_in_cmd = impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id];
        impl(_this)->event_cb_fn_t->switch_merge_cb_fn(impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                                   impl(_this)->m_sorted_port[ impl(_this)->m_next_port_id],
                                   SM_SV_PMS_EVENT_SWITCH_FAIL,
                                   _this);
        return;
    }
    if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER &&
        impl(_this)->m_port[impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id]].m_bp_state == BP_STATE_ONLY_DISCHARGING)
    {
        pms_set_bp_cmd(_this,
                       BP_CMD_STANDBY,
                       impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id]);
        LOG_INF(TAG, "BP_CMD_STANDBY - port %d", impl(_this)->m_port_in_cmd);
    }
}
static void pms_merge_handle(sm_sv_pms_t* _this){
    if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER){
        if(impl(_this)->m_pms_state == PMS_ST_DISCHARGING){
            impl(_this)->m_port_in_cmd = (int32_t)impl(_this)->m_sorted_port[impl(_this)->m_next_port_id];
            impl(_this)->m_sdo_cmd = BP_CMD_DISCHARGE;
            impl(_this)->m_sdo_st = SM_BP_CMD_NULL;
            sm_sv_bp_set_cmd(impl(_this)->m_bp,
                             impl(_this)->m_port_in_cmd,
                             BP_CMD_DISCHARGE,
                             NULL,
                             pms_cmd_cb_handle,
                             _this);
//            elapsed_timer_reset(&impl(_this)->m_switch_merge_timeout);
        }
    }

    // Check SDO
    if (impl(_this)->m_sdo_st == SM_BP_CMD_SUCCESS && impl(_this)->m_sdo_cmd == BP_CMD_DISCHARGE)
    {
            impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
            impl(_this)->event_cb_fn_t->switch_merge_cb_fn(impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                                       impl(_this)->m_sorted_port[ impl(_this)->m_next_port_id],
                                       SM_SV_PMS_EVENT_MERGE_SUCCESS,
                                       _this);
            impl(_this)->m_impl_port_id = impl(_this)->m_next_port_id;
            pms_reset_sdo_status(_this);
    }
    else if(impl(_this)->m_sdo_st == SM_BP_CMD_FAILURE )
    {
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        impl(_this)->event_cb_fn_t->switch_merge_cb_fn(impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                                   impl(_this)->m_sorted_port[ impl(_this)->m_next_port_id],
                                   SM_SV_PMS_EVENT_MERGE_FAIL,
                                   _this);
        pms_reset_sdo_status(_this);
    }
}
static void pms_recovery_handle(sm_sv_pms_t* _this){
    if (impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id] == PORT_TRASH) return;

    if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER){
        impl(_this)->m_sdo_cmd = BP_CMD_DISCHARGE;
        impl(_this)->m_sdo_st = SM_BP_CMD_NULL;
        impl(_this)->m_port_in_cmd = (int32_t) impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id];
        LOG_DBG(TAG, "RECOVERY SET CMD :SDO %d, PORT: %d", impl(_this)->m_sdo_cmd, impl(_this)->m_port_in_cmd);
        sm_sv_bp_set_cmd(impl(_this)->m_bp,
                         impl(_this)->m_port_in_cmd,
                         BP_CMD_DISCHARGE,
                         NULL,
                         pms_cmd_cb_handle,
                         _this);
    }
    if (impl(_this)->m_sdo_st == SM_BP_CMD_SUCCESS &&
        impl(_this)->m_sdo_cmd == BP_CMD_DISCHARGE){
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        LOG_DBG(TAG, "RECOVERY -> NO SWITCH");
        pms_reset_sdo_status(_this);
        elapsed_timer_reset(&impl(_this)->m_switch_merge_timeout);
    }
    else if (impl(_this)->m_sdo_st == SM_BP_CMD_FAILURE &&
             impl(_this)->m_sdo_cmd == BP_CMD_DISCHARGE){
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        LOG_DBG(TAG, "RECOVERY -> NO SWITCH");
        pms_reset_sdo_status(_this);
    }
}
static void pms_force_switch_handle(sm_sv_pms_t* _this){
    if (impl(_this)->m_sdo_st == SM_BP_CMD_SUCCESS && impl(_this)->m_sdo_cmd == BP_CMD_STANDBY){
        pms_reset_sdo_status(_this);
    } else if (impl(_this)->m_sdo_st == SM_BP_CMD_FAILURE && impl(_this)->m_sdo_cmd == BP_CMD_STANDBY){
        LOG_ERR(TAG, "FORCE SWITCH FAIL !");
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
        pms_reset_sdo_status(_this);
        return;
    }
    if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER){
        for (int i = 0; i < impl(_this)->m_config->m_max_bp_num; ++i) {
            if (i != impl(_this)->m_impl_port_id &&
                impl(_this)->m_sorted_port[i] != PORT_TRASH &&
                impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_state == BP_STATE_DISCHARGING &&
                impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_switch_merge_st == PORT_ST_ENABLE)
            {
                pms_set_bp_cmd(_this, BP_CMD_STANDBY, impl(_this)->m_sorted_port[i]);
                return;
            }
        }
    }

    if (impl(_this)->m_impl_port_id == PORT_TRASH ||
        impl(_this)->m_port[impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id]].m_switch_merge_st != PORT_ST_FORCED_SWITCH)
    {
        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
    }
}
static void pms_force_merge_handle(sm_sv_pms_t* _this){
    if (impl(_this)->m_impl_port_id == PORT_TRASH ||
            impl(_this)->m_port[impl(_this)->m_impl_port_id].m_switch_merge_st != PORT_ST_FORCED_MERGE)
    {
        impl(_this)->m_pms_state = SM_SV_PMS_ST_NO_SWITCH;
    }
}

static int8_t pms_rescan_impl_port(sm_sv_pms_t* _this){
    int8_t new_impl_port_id = -1;
    for (uint8_t i = 0; i < impl(_this)->m_config->m_max_bp_num; ++i) {
        if  (impl(_this)->m_sorted_port[i] == PORT_TRASH) continue;
        if  (impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_connection_st == PORT_ST_ENABLE){
                new_impl_port_id = (int8_t) i;
                return new_impl_port_id;
        }
    }
    return new_impl_port_id;
}
static void pms_disable_port_operation(sm_sv_pms_t* _this, uint8_t _id){
    if (_id < 0 ||
        _id > impl(_this)->m_config->m_max_bp_num ||
        impl(_this)->m_port[_id].m_connection_st == PORT_ST_DISABLE)
        return;

    int32_t impl_port = impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id];
    if (impl_port == _id)
    {
        impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id] = PORT_TRASH;
        impl(_this)->m_impl_port_id = PORT_TRASH;
    }
    else{
        for (uint8_t i=0; i< impl(_this)->m_config->m_max_bp_num; ++i){
            if (impl(_this)->m_sorted_port[i] == _id)
                impl(_this)->m_sorted_port[i] = PORT_TRASH;
        }
    }
    impl(_this)->m_impl_port_id = pms_rescan_impl_port(_this);
    if (impl(_this)->m_impl_port_id == PORT_TRASH)
        impl(_this)->m_pms_state = PMS_ST_INIT;
}
static void pms_set_bp_cmd(sm_sv_pms_t *_this, SM_BP_CMD _cmd, uint8_t _id){
    impl(_this)->m_sdo_cmd = _cmd;
    impl(_this)->m_sdo_st = SM_BP_CMD_NULL;
    impl(_this)->m_port_in_cmd = _id;
    sm_sv_bp_set_cmd(impl(_this)->m_bp,
                     impl(_this)->m_port_in_cmd,
                     _cmd,
                     NULL,
                     pms_cmd_cb_handle,
                     _this);
    LOG_DBG(TAG, "PMS SET CMD :SDO %d, PORT: %d", impl(_this)->m_sdo_cmd, impl(_this)->m_port_in_cmd);
}
static int8_t is_port_in_sorted_list(sm_sv_pms_t *_this, int8_t _id){
    int8_t result = -1;
    for (int8_t i = 0; i < impl(_this)->m_config->m_max_bp_num; ++i) {
        if (impl(_this)->m_sorted_port[i] == _id) result = i;
    }
    return result;
}
static void pms_check_port_operation(sm_sv_pms_t *_this){
    if (impl(_this)->m_switch_merge_st != SM_SV_PMS_ST_NO_SWITCH) return;
    // Check SDO
    if (impl(_this)->m_sdo_st == SM_BP_CMD_SUCCESS &&
        impl(_this)->m_sdo_cmd == BP_CMD_STANDBY){
        pms_disable_port_operation(_this, impl(_this)->m_port_in_cmd);
        pms_reset_sdo_status(_this);
    } else if (impl(_this)->m_sdo_st == SM_BP_CMD_FAILURE &&
              impl(_this)->m_sdo_cmd == BP_CMD_STANDBY) {
        pms_reset_sdo_status(_this);
    }
    else if (impl(_this)->m_sdo_cmd == BP_CMD_NUMBER)
    {
        for (int i = 0; i < impl(_this)->m_config->m_max_bp_num; ++i)
        {
            if (impl(_this)->m_port[i].m_connection_st == PORT_ST_DISABLE) // Port's empty
                continue;
            if (impl(_this)->m_pms_state == PMS_ST_DISCHARGING)
            {
                if (impl(_this)->m_port[i].m_switch_merge_st == PORT_ST_DISABLE &&
                    impl(_this)->m_port[i].m_action_st == PORT_ST_ENABLE)
                {
                    /*pms_disable_port_operation(_this, impl(_this)->m_port_in_cmd);*/
                    pms_set_bp_cmd(_this, BP_CMD_STANDBY, i);
                }
                else if (impl(_this)->m_port[i].m_switch_merge_st == PORT_ST_ENABLE &&
                     impl(_this)->m_port[i].m_action_st == PORT_ST_DISABLE &&
                    is_port_in_sorted_list(_this, i) == -1)
                {
                    for (uint8_t j=0; j< impl(_this)->m_config->m_max_bp_num; ++j){
                        if (impl(_this)->m_sorted_port[j] != PORT_TRASH) continue;
                        impl(_this)->m_sorted_port[j] = (int8_t) i;
                        if (impl(_this)->m_impl_port_id == PORT_TRASH)
                            impl(_this)->m_impl_port_id = (int8_t) j;
                        return;
                    }
                }
            }
            else{ // OUT OF BP POWER
                if (impl(_this)->m_port[i].m_switch_merge_st == PORT_ST_ENABLE &&
                    impl(_this)->m_port[i].m_action_st == PORT_ST_DISABLE)
                {
                    impl(_this)->m_pms_state = PMS_ST_DISCHARGING;
                    impl(_this)->m_impl_port_id = 0;
                    impl(_this)->m_sorted_port[0] = i;
                    return;
                }
            }
        }
    }
}
static void pms_switch_merge_process(sm_sv_pms_t* _this){
    pms_sort_port(_this);
    switch (impl(_this)->m_switch_merge_st){
        case SM_SV_PMS_ST_NO_SWITCH:
//            LOG_DBG(TAG, "NO SWITCH ");
            elapsed_timer_reset(&impl(_this)->m_switch_merge_timeout);
            impl(_this)->m_next_port_id = PORT_TRASH;

            if (impl(_this)->m_impl_port_id == PORT_TRASH)
                impl(_this)->m_impl_port_id = pms_rescan_impl_port(_this);

//                LOG_DBG(TAG, "NO SWITCH -> NO SWITCH");
                if(impl(_this)->m_pms_state == PMS_ST_DISCHARGING){
                if (impl(_this)->m_port[impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id]].m_connection_st == PORT_ST_ENABLE &&
                    impl(_this)->m_port[impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id]].m_bp_data->m_state == BP_STATE_STANDBY){
                        impl(_this)->m_sdo_cmd = BP_CMD_NUMBER;
                        impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_RECOVERY;
                        impl(_this)->m_port_in_cmd = (int32_t) impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id];
//                        LOG_DBG(TAG, "NO SWITCH -> RECOVERY SWITCH");
                        return;
                }
                for (int8_t i = 0; i < impl(_this)->m_config->m_max_bp_num ; ++i) {
                    if (impl(_this)->m_sorted_port[i] == PORT_TRASH)
                        continue;
                    impl(_this)->m_next_port_id = i;
                    if (impl(_this)->m_next_port_id == impl(_this)->m_impl_port_id ||
                        impl(_this)->m_port[impl(_this)->m_sorted_port[i]].m_bp_data->m_state == BP_STATE_DISCHARGING)
                        continue;
                    pms_update_switch_discharge_st(_this);
                    break;
                }
            }
            LOG_INF(TAG, "Discharging impl index: %d is port %d - next index : %d is port %d",
                    impl(_this)->m_impl_port_id, impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                    impl(_this)->m_next_port_id, impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]);
            pms_reset_sdo_status(_this);
            break;
        case SM_SV_PMS_ST_PRE_SWITCH:
            if (!elapsed_timer_get_remain(&impl(_this)->m_switch_merge_timeout)){
                impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
                pms_reset_sdo_status(_this);
            }
            LOG_INF(TAG, "Impl index: %d is port %d - next index : %d is port %d",
                    impl(_this)->m_impl_port_id, impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                    impl(_this)->m_next_port_id, impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]);
            LOG_DBG(TAG, "PRE SWITCH");// off charge-fet impl-ports
            pms_pre_switch_handle(_this);
            break;
        case SM_SV_PMS_ST_MAIN_SWITCH:
            if (!elapsed_timer_get_remain(&impl(_this)->m_switch_merge_timeout)){
                impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
                pms_reset_sdo_status(_this);
            }
            LOG_INF(TAG, "Impl index: %d is port %d - next index : %d is port %d",
                    impl(_this)->m_impl_port_id, impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                    impl(_this)->m_next_port_id, impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]);
            LOG_DBG(TAG, "MAIN SWITCH");// on all fet next-port
            pms_main_switch_handle(_this);
            break;
        case SM_SV_PMS_ST_FINISH_SWITCH:
            if (!elapsed_timer_get_remain(&impl(_this)->m_switch_merge_timeout)){
                impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
                pms_reset_sdo_status(_this);
            }
            LOG_INF(TAG, "Impl index: %d is port %d - next index : %d is port %d",
                    impl(_this)->m_impl_port_id, impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                    impl(_this)->m_next_port_id, impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]);
            LOG_DBG(TAG, "FINISH SWITCH");// off all fet impl-ports
            pms_finish_switch_handle(_this);
            break;
        case SM_SV_PMS_ST_MERGE:
            if (!elapsed_timer_get_remain(&impl(_this)->m_switch_merge_timeout)){
                impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
                pms_reset_sdo_status(_this);
            }
            LOG_INF(TAG, "Impl index: %d is port %d - next index : %d is port %d",
                    impl(_this)->m_impl_port_id, impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                    impl(_this)->m_next_port_id, impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]);
            LOG_DBG(TAG, "MERGE SWITCH");// on all fet next-port
            pms_merge_handle(_this);
            break;
        case SM_SV_PMS_ST_RECOVERY:
            if (!elapsed_timer_get_remain(&impl(_this)->m_switch_merge_timeout)){
                impl(_this)->m_switch_merge_st = SM_SV_PMS_ST_NO_SWITCH;
                pms_reset_sdo_status(_this);
            }
            LOG_INF(TAG, "Impl index: %d is port %d - next index : %d is port %d",
                    impl(_this)->m_impl_port_id, impl(_this)->m_sorted_port[impl(_this)->m_impl_port_id],
                    impl(_this)->m_next_port_id, impl(_this)->m_sorted_port[impl(_this)->m_next_port_id]);
            LOG_DBG(TAG, "RECOVERY SWITCH");
            pms_recovery_handle(_this);
            break;
        case SM_SV_PMS_ST_FORCE_SWITCH:
            pms_force_switch_handle(_this);
            break;
    }
}
void pms_bp_connected_cb_fn(int32_t _id, const char* _sn, int32_t _soc, void* _arg){
    if (_id == PORT_TRASH || _id >= impl(_arg)->m_config->m_max_bp_num) return;
    if (impl(_arg)->m_port[_id].m_connection_st != PORT_ST_DISABLE) return;

    impl(_arg)->m_port[_id].m_connection_st     = PORT_ST_ENABLE;
    impl(_arg)->m_port[_id].m_action_st         = PORT_ST_DISABLE;
    impl(_arg)->m_port[_id].m_switch_merge_st   = PORT_ST_ENABLE;
    impl(_arg)->m_port[_id].m_bp_state          = BP_STATE_STANDBY;

    for (uint8_t i=0; i< impl(_arg)->m_config->m_max_bp_num; ++i){
        if (impl(_arg)->m_sorted_port[i] != PORT_TRASH) continue;
        if (_id < impl(_arg)->m_config->m_max_bp_num)
        {
            impl(_arg)->m_sorted_port[i] = (int8_t) _id;
            if (impl(_arg)->m_impl_port_id == PORT_TRASH)
                impl(_arg)->m_impl_port_id = (int8_t) i;
            if (impl(_arg)->m_pms_state == PMS_ST_INIT)
                impl(_arg)->m_pms_state = PMS_ST_DISCHARGING;
            return;
        }
    }
}
void pms_bp_disconnected_cb_fn(int32_t _id, const char* _sn, void* _arg){
    if (_id == PORT_TRASH) return;
    impl(_arg)->m_port[_id].m_connection_st    = PORT_ST_DISABLE;
    impl(_arg)->m_port[_id].m_action_st        = PORT_ST_DISABLE;
    impl(_arg)->m_port[_id].m_switch_merge_st  = PORT_ST_ENABLE;
    impl(_arg)->m_port[_id].m_valid_st         = PORT_ST_NO_CHECK;
    impl(_arg)->m_port[_id].m_bp_state         = BP_STATE_IDLE;

    int32_t impl_port = impl(_arg)->m_sorted_port[impl(_arg)->m_impl_port_id];
    if (impl_port == _id)
    {
        impl(_arg)->m_sorted_port[impl(_arg)->m_impl_port_id] = PORT_TRASH;
        impl(_arg)->m_impl_port_id = PORT_TRASH;
    }
    else{
        for (uint8_t i=0; i< impl(_arg)->m_config->m_max_bp_num; ++i){
            if (impl(_arg)->m_sorted_port[i] == _id)
                impl(_arg)->m_sorted_port[i] = PORT_TRASH;
        }
    }
    impl(_arg)->m_impl_port_id = pms_rescan_impl_port(_arg);
    if (impl(_arg)->m_impl_port_id == PORT_TRASH)
        impl(_arg)->m_pms_state = PMS_ST_INIT;
}
void pms_bp_update_data_cb_fn(int32_t _id, const sm_bp_data_t* _data, void* _arg){
    sm_bp_clone_data(impl(_arg)->m_port[_id].m_bp_data, _data);
    if (_data->m_state != BP_STATE_STANDBY)
        impl(_arg)->m_port[_id].m_action_st = PORT_ST_ENABLE;
    else
        impl(_arg)->m_port[_id].m_action_st = PORT_ST_DISABLE;
//    LOG_DBG(TAG,"Receive BP data %d : %d %d", _id, _data->m_state, _data->m_soc);
}
static void sm_pms_read_data_rx(sm_sv_pms_t *_this){
    /*
     * Trigger by changing activity mode signal
     */
    if(impl(_this)->m_is_new_data){
        if (impl(_this)->m_pms_state == PMS_ST_INIT){
            impl(_this)->m_pms_state = impl(_this)->m_pms_data_rx->m_activity_st;
        }
        if (impl(_this)->m_switch_merge_st == SM_SV_PMS_ST_NO_SWITCH &&
                impl(_this)->m_pms_state != impl(_this)->m_pms_data_rx->m_activity_st){
            impl(_this)->m_pms_state = impl(_this)->m_pms_data_rx->m_activity_st;
            impl(_this)->m_impl_port_id = impl(_this)->m_pms_data_rx->m_impl_port_id;
        }
    }

    impl(_this)->m_is_new_data = 0;
}
static void sm_pms_build_data_tx(sm_sv_pms_t *_this){
    for (int i = 0; i < impl(_this)->m_config->m_max_bp_num; ++i) {
        impl(_this)->m_pms_data_tx->m_sorted_port[i] = impl(_this)->m_sorted_port[i];
    }
    impl(_this)->m_pms_data_tx->m_impl_port_id      = impl(_this)->m_impl_port_id;
    impl(_this)->m_pms_data_tx->m_switch_merge_st   = impl(_this)->m_switch_merge_st;
    impl(_this)->m_pms_data_tx->m_activity_st       = impl(_this)->m_pms_state;
}
int32_t sm_sv_pms_process(sm_sv_pms_t* _this){
    if (_this == NULL) {
        return -1;
    }
    pms_check_port_operation(_this);
    switch (impl(_this)->m_pms_state) {
        case PMS_ST_DISCHARGING:
//            LOG_DBG(TAG, "PMS_DISCHARGING");
            pms_switch_merge_process(_this);
//            impl(_this)->event_cb_fn_t->update_data_cb_fn(impl(_this)->m_pms_data_tx, impl(_this)->m_event_arg);
            break;
        case PMS_ST_CHARGING:
//            LOG_DBG(TAG, "PMS_CHARGING");
            break;
        case PMS_ST_INIT:
//            LOG_DBG(TAG, "PMS_INIT");
            for (int i = 0; i < impl(_this)->m_config->m_max_bp_num; ++i) {
                impl(_this)->m_sorted_port[i] = -1;
                impl(_this)->m_port[i].m_action_st = PORT_ST_DISABLE;
            }
            impl(_this)->m_impl_port_id = PORT_TRASH;
            impl(_this)->m_next_port_id = PORT_TRASH;
            pms_reset_sdo_status(_this);
            break;
    }
/*    LOG_DBG(TAG, "SDO CMD: %d SDO ST: %d PORT CMD: %d ", impl(_this)->m_sdo_cmd, impl(_this)->m_sdo_st, impl(_this)->m_port_in_cmd);
    sm_pms_build_data_tx(_this);
    sm_pms_read_data_rx(_this);
    sm_pms_est_process(impl(_this)->m_est, impl(_this)->m_port);
    impl(_this)->event_cb_fn_t->update_est_data_cb_fn(impl(_this)->m_est, impl(_this)->m_event_arg);*/
    return 0;
}
int32_t sm_sv_pms_get_estimate_data(sm_sv_pms_t* _this, est_data_t* _est_data){
    if (_this == NULL) return -1;
    if (est_data_cpy(impl(_this)->m_est, _est_data))
        return 0;
    return -1;
}
int32_t sm_sv_pms_receive_data(sm_sv_pms_t * _this, sm_pms_data_t *_new_data){
    if (_this == NULL) return -1;
    pms_clone_data(impl(_this)->m_pms_data_rx, _new_data);
    impl(_this)->m_is_new_data = 1;
    return 0;
}

