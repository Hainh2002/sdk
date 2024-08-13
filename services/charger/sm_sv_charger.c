//
// Created by DELL 5425 on 3/13/2024.
//

#ifndef EV_SDK_SM_SV_CHARGER_C_H
#define EV_SDK_SM_SV_CHARGER_C_H

#include <stdlib.h>
#include "sm_utils.h"

#include "sm_sv_charger.h"
#include "sm_sv_bp.h"
#include "sm_logger.h"

#include "sm_elapsed_timer.h"

static const char* TAG = "SV_CHARGER";

#define _impl(x) ((sm_sv_charger_impl_t*)(x))

#define BP_NUMBER_CHARGER_SUPPORT           3

#define CHARGER_IDLE_TIMEOUT                3000
#define CHARGER_BP_CMD_WAITING_TIMEOUT      3000
#define CHARGER_FORCE_TIMEOUT               5000
#define CHARGER_SWITCH_MERGE_TIMEOUT        1000

typedef enum {
    SM_CHARGE_SWITCH,
    SM_CHARGE_MERGE,
    SM_CHARGE_NULL,
} SM_CHARGER_OPERATION;
typedef enum {
     SM_CHARGER_IDLE,
    SM_START_CHARGING,
    SM_WAITING_START_CHARGING,
    SM_CHARGED,
    SM_STOP_CHARGING,
    SM_NOT_CHARGED,
    SM_MERGE,
    SM_SWITCH,
    SM_PRE_FORCED,
    SM_FORCED,
} SM_CHARGER_STATE_MACHINE;

typedef struct {
    sm_sv_charger_prof_t 	    m_prof;
    sm_sv_charger_if_t*         m_if;
    sm_sv_bp_t*                 m_bpm;

    sm_sv_charger_data_t        m_data;
    uint8_t                     m_plugged_in;

    SM_CHARGER_STATE_MACHINE    m_state;
    elapsed_timer_t             m_state_timeout;
    elapsed_timer_t             m_switch_merge_timeout;
    int32_t                     m_waiting_bp;
    int32_t                     m_forced_bp;
    int32_t                     m_bp_is_enable[BP_NUMBER_CHARGER_SUPPORT];
    int32_t                     m_waiting_bp_list[BP_NUMBER_CHARGER_SUPPORT];
    int32_t                     m_charging_bp_list[BP_NUMBER_CHARGER_SUPPORT];
    int32_t                     m_charging_bp_number;
    int32_t                     m_waiting_bp_number;

    int32_t                     m_err;

    void*                       m_event_arg;
    const sm_sv_charger_event_cb_fn_t *m_event_cb;
} sm_sv_charger_impl_t;

static uint8_t is_charger_plugged_in(sm_sv_charger_t* _this);
static void sm_charger_align_left_bp_list(sm_sv_charger_t *_this);

static sm_sv_charger_impl_t g_charger_default = {
        .m_bpm = NULL,
        .m_if = NULL,
        .m_plugged_in = 0,
        .m_event_arg = NULL,
        .m_event_cb = NULL
};
void sm_sv_charger_on_bp_connected(int32_t _id, const char* _sn, int32_t _soc, void* _arg){
    if (!_arg) return;
    uint8_t index = 0;
    sm_charger_align_left_bp_list(_arg);
    _impl(_arg)->m_waiting_bp_list[_impl(_arg)->m_waiting_bp_number] = _id;
    _impl(_arg)->m_waiting_bp_number++;
    _impl(_arg)->m_bp_is_enable[_id] = 1;
}
void sm_sv_charger_on_bp_disconnected(int32_t _id, const char* _sn, void* _arg){
    if (!_arg) return;

    for (int i = 0; i < BP_NUMBER_CHARGER_SUPPORT; ++i) {
        if (_impl(_arg)->m_waiting_bp_list[i] == _id) {
            _impl(_arg)->m_waiting_bp_number--;
            _impl(_arg)->m_waiting_bp_list[i] = -1;
        }
        if (_impl(_arg)->m_charging_bp_list[i] == _id) {
            _impl(_arg)->m_charging_bp_number--;
            _impl(_arg)->m_charging_bp_list[i] = -1;
        }
        if (_impl(_arg)->m_charging_bp_number == 0 && _impl(_arg)->m_plugged_in == SM_CHARGER_PLUGGED_IN)
            _impl(_arg)->m_state = SM_CHARGER_IDLE;
    }
    if (_impl(_arg)->m_forced_bp == _id) _impl(_arg)->m_forced_bp = -1;
    _impl(_arg)->m_bp_is_enable[_id] = 1;
}

sm_sv_bp_event_cb_t g_charger_bp_event_cb = {
        .on_bp_connected = sm_sv_charger_on_bp_connected,
        .on_bp_disconnected = sm_sv_charger_on_bp_disconnected,
        .on_bp_update_data = NULL,
};
sm_sv_charger_t* sm_sv_charger_create_default(sm_sv_charger_prof_t* _prof, sm_sv_charger_if_t* _if, sm_sv_bp_t* _bpm){
    g_charger_default.m_if = (sm_sv_charger_if_t*) _if;
    g_charger_default.m_bpm = _bpm;
//    sm_sv_bp_reg_event(g_charger_default.m_bpm, &g_charger_bp_event_cb,&g_charger_default);
    sm_sv_charger_set_profile(&g_charger_default, _prof);
    g_charger_default.m_plugged_in =SM_CHARGER_UNPLUGGED_IN;
    for (int i = 0; i < BP_NUMBER_CHARGER_SUPPORT; ++i) {
        g_charger_default.m_charging_bp_list[i] = -1;
        g_charger_default.m_waiting_bp_list[i] = -1;
        g_charger_default.m_bp_is_enable[i] = 1;
    }
    g_charger_default.m_charging_bp_number = 0;
    g_charger_default.m_waiting_bp_number = 0;

    g_charger_default.m_forced_bp = -1;
    elapsed_timer_resetz(&g_charger_default.m_state_timeout, CHARGER_BP_CMD_WAITING_TIMEOUT);
    elapsed_timer_resetz(&g_charger_default.m_switch_merge_timeout, CHARGER_SWITCH_MERGE_TIMEOUT);
    return &g_charger_default;
}

sm_sv_charger_t* sm_sv_charger_create(sm_sv_charger_prof_t* _prof, sm_sv_charger_if_t* _if, sm_sv_bp_t* _bpm){
    sm_sv_charger_impl_t *charger = malloc(sizeof(sm_sv_charger_impl_t));
    if (!charger) {
        return NULL;
    }
    charger->m_if = (sm_sv_charger_if_t*) _if;
    charger->m_bpm = _bpm;

    sm_sv_charger_set_profile(charger, _prof);

    return charger;
}

int32_t sm_sv_charger_destroy(sm_sv_charger_t* _this){
    if (!_this) {
        return -1;
    }
    _impl(_this)->m_if = NULL;

    free(_this);
    _this = NULL;
    return 0;
}

int32_t sm_sv_charger_reg_event(sm_sv_charger_t* _this,
                                const sm_sv_charger_event_cb_fn_t* _cb_fn, void* _arg){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_event_cb = _cb_fn;
    _impl(_this)->m_event_arg = _arg;
    return 0;
}

int32_t sm_sv_charger_set_profile(sm_sv_charger_t* _this, const sm_sv_charger_prof_t* _prof){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_prof.m_max_volt = _prof->m_max_volt;
    _impl(_this)->m_prof.m_min_volt = _prof->m_min_volt;
    _impl(_this)->m_prof.m_max_cur = _prof->m_max_cur;
    _impl(_this)->m_prof.m_max_temp = _prof->m_max_temp;
    _impl(_this)->m_prof.m_power_stable_time = _prof->m_power_stable_time;
    return 0;
}

const sm_sv_charger_prof_t* sm_sv_charger_get_profile(sm_sv_charger_t* _this){
    if(!_this){
        return NULL;
    }
    return &_impl(_this)->m_prof;
}

int32_t sm_sv_charger_get_bp_num(sm_sv_charger_t* _this){
    if (_this == NULL) {
        return -1;
    }
    return _impl(_this)->m_charging_bp_number;
}

int32_t sm_sv_charger_get_cur(sm_sv_charger_t* _this){
    if (_this == NULL) 
        return -1;
    return _impl(_this)->m_data.m_cur;
}

int32_t sm_sv_charger_get_volt(sm_sv_charger_t* _this){
    if (_this == NULL)
        return -1;
    return _impl(_this)->m_data.m_vol;
}

int32_t sm_sv_charger_get_state(sm_sv_charger_t* _this){
    if (_this == NULL)
        return -1;
    return (int32_t) _impl(_this)->m_data.m_state;
}
static void sm_charger_move_bp_to_waiting_list(sm_sv_charger_t* _this, int32_t _bp_id){
    // remove from charging list
    for (int i = 0; i < BP_NUMBER_CHARGER_SUPPORT; ++i) {
        if (_bp_id == _impl(_this)->m_charging_bp_list[i]){
            _impl(_this)->m_charging_bp_list[i] = -1;
            _impl(_this)->m_charging_bp_number--;
            break;
        }
    }
    // add to waiting list
    for (int i = 0; i < BP_NUMBER_CHARGER_SUPPORT; ++i) {
        if (_impl(_this)->m_waiting_bp_list[i] == -1){
            _impl(_this)->m_waiting_bp_list[i] = _bp_id;
            _impl(_this)->m_waiting_bp_number++;
            break;
        }
    }

}
static void sm_charger_move_bp_to_charging_list(sm_sv_charger_t* _this, int32_t _bp_id){
    // remove from waiting list
    for (int i = 0; i < BP_NUMBER_CHARGER_SUPPORT; ++i) {
        if (_bp_id == _impl(_this)->m_waiting_bp_list[i]){
            _impl(_this)->m_waiting_bp_list[i] = -1;
            _impl(_this)->m_waiting_bp_number--;
            break;
        }
    }

    // add to charging list
    for (int i = 0; i < BP_NUMBER_CHARGER_SUPPORT; ++i) {
        if (_impl(_this)->m_charging_bp_list[i] == -1){
            _impl(_this)->m_charging_bp_list[i] = _bp_id;
            _impl(_this)->m_charging_bp_number++;
            break;
        }
    }
}
static void sm_charging_bp_cmd_handle(int32_t _id, SM_BP_CMD _cmd, int32_t _err, void* _data, void* _arg){
    LOG_DBG(TAG, "ID: %d, CMD: %d, ERR: %d", _id, _cmd, _err);
    if(!_arg){
        return;
    }
    if (!_err){
        int8_t i;
        switch (_cmd) {
            case BP_CMD_CHARGE:
                sm_charger_move_bp_to_charging_list(_arg, _id);
                break;
            case BP_CMD_STANDBY:
                sm_charger_move_bp_to_waiting_list(_arg, _id);
                if (_impl(_arg)->m_state == SM_FORCED &&
                    _impl(_arg)->m_forced_bp == _id){
                    _impl(_arg)->m_forced_bp = -1;
                    _impl(_arg)->m_state = SM_CHARGER_IDLE;
                    LOG_DBG(TAG, "RELEASE SUCCESS");
                }
                if (_impl(_arg)->m_charging_bp_number == 0 && _impl(_arg)->m_forced_bp == -1)
                    _impl(_arg)->m_state = SM_NOT_CHARGED;
                break;
            default:
                break;
        }
        if (_impl(_arg)->m_waiting_bp != -1)
            _impl(_arg)->m_waiting_bp = -1;
    }else{
        switch (_cmd) {
            case BP_CMD_CHARGE:
                if (_impl(_arg)->m_state == SM_WAITING_START_CHARGING){
                    _impl(_arg)->m_state = SM_NOT_CHARGED;
                    LOG_ERR(TAG, "WAITING START CHARGING FAIL!!!");
                }
                if (_impl(_arg)->m_state == SM_FORCED &&
                    _impl(_arg)->m_forced_bp == _id) {
                    _impl(_arg)->m_forced_bp = -1;
                    _impl(_arg)->m_state = SM_STOP_CHARGING;
                    LOG_ERR(TAG, "FORCE FAIL");
                }
                break;
            case BP_CMD_STANDBY:
               if (_impl(_arg)->m_state == SM_FORCED &&
                    _impl(_arg)->m_forced_bp == _id){
                    _impl(_arg)->m_forced_bp = -1;
                   LOG_ERR(TAG, "RELEASE FAIL");
                }
               if (_impl(_arg)->m_bp_is_enable[_id] == 0){
                   _impl(_arg)->m_bp_is_enable[_id] = 1;
                   LOG_ERR(TAG, "DISABLE BP %d FAIL", _id);
               }
                break;
            default:
                break;
        }
        _impl(_arg)->m_waiting_bp = -1;
    }
    sm_charger_align_left_bp_list(_arg);
    sm_sv_bp_reset_current_cmd(_impl(_arg)->m_bpm);
}

int32_t sm_sv_charger_force(sm_sv_charger_t* _this, uint8_t _bp_id){
    if (_this == NULL) {
        return -1;
    }
    if (_impl(_this)->m_state == SM_START_CHARGING ||
        _impl(_this)->m_state == SM_MERGE ||
        _impl(_this)->m_state == SM_SWITCH ||
        _impl(_this)->m_state == SM_STOP_CHARGING ||
        _impl(_this)->m_bp_is_enable[_bp_id] == 0)
        return -1;
    if (_impl(_this)->m_state == SM_FORCED ||
        _impl(_this)->m_state == SM_PRE_FORCED ||
        _impl(_this)->m_forced_bp != -1){
        return _impl(_this)->m_forced_bp + 5;
    }

    if (sm_sv_bp_is_connected(_impl(_this)->m_bpm, _bp_id) && is_charger_plugged_in(_this)){
        LOG_DBG(TAG, "PRE_FORCED");
        _impl(_this)->m_state = SM_PRE_FORCED;
        _impl(_this)->m_forced_bp = _bp_id;
        return 0;
    }
    return -1;
}

int32_t sm_sv_charger_release(sm_sv_charger_t* _this){
    if (_this == NULL) {
        return -1;
    }
    if (_impl(_this)->m_state == SM_FORCED){
        int32_t forced_bp_id = _impl(_this)->m_forced_bp;
        if (sm_sv_bp_is_connected(_impl(_this)->m_bpm, forced_bp_id)){
            _impl(_this)->m_waiting_bp = forced_bp_id;
            return sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                             _impl(_this)->m_waiting_bp,
                            BP_CMD_STANDBY,
                            NULL,
                            sm_charging_bp_cmd_handle,
                            _this);
        }
//        _impl(_this)->m_state = SM_CHARGER_IDLE;
    }
    return -1;
}

int32_t sm_sv_charger_disable_bp(sm_sv_charger_t* _this, uint8_t _bp_id){
    if(!_this || _bp_id < 0 || _bp_id > BP_NUMBER_CHARGER_SUPPORT) return -1;
    if (!sm_sv_bp_is_connected(_impl(_this)->m_bpm, _bp_id)) return -1;
    if (_impl(_this)->m_bp_is_enable[_bp_id] == 0) return 0;
    const sm_bp_data_t *bp_data = NULL;
    bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _bp_id);
    if (bp_data->m_state != BP_STATE_STANDBY){
        _impl(_this)->m_bp_is_enable[_bp_id] = 0;
        if (_impl(_this)->m_forced_bp == _bp_id){
            sm_sv_charger_release(_this);
        }else{
            _impl(_this)->m_waiting_bp = _bp_id;
            if (sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                                    _impl(_this)->m_waiting_bp,
                                    BP_CMD_STANDBY,
                                    NULL,
                                    sm_charging_bp_cmd_handle,
                                    _this))
                return -1;
        }
//        _impl(_this)->m_bp_is_enable[_bp_id] = 0;
        return 0;
    }
    return -1;
}

int32_t sm_sv_charger_enable_bp(sm_sv_charger_t* _this, uint8_t _bp_id){
    if(!_this || _bp_id < 0 || _bp_id > BP_NUMBER_CHARGER_SUPPORT) return -1;
    if (!sm_sv_bp_is_connected(_impl(_this)->m_bpm, _bp_id)) return -1;
    if (_impl(_this)->m_bp_is_enable[_bp_id] == 1) return 0;
    const sm_bp_data_t *bp_data = NULL;
    bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _bp_id);
    if (bp_data->m_state == BP_STATE_STANDBY){
        _impl(_this)->m_bp_is_enable[_bp_id] = 1;
        return 0;
    }
    return -1;
}

static uint8_t is_charger_plugged_in(sm_sv_charger_t* _this){
    if(!_impl(_this)->m_if->get_charger_vol_fn_t()){
        return SM_CHARGER_UNPLUGGED_IN;
    }

    if (_impl(_this)->m_if->get_charger_vol_fn_t() > _impl(_this)->m_prof.m_min_volt &&
        _impl(_this)->m_if->get_charger_vol_fn_t() < _impl(_this)->m_prof.m_max_volt) {
        return SM_CHARGER_PLUGGED_IN;
    }else {
        return SM_CHARGER_UNPLUGGED_IN;
    }
}

static int32_t sm_charger_find_first_bp(sm_sv_charger_t* _this){
    int32_t bp_list[BP_NUMBER_CHARGER_SUPPORT];

    int32_t num = sm_sv_bp_get_connected_bp(_impl(_this)->m_bpm, bp_list);
    if(num <= 0){
        return -1;
    }

    const sm_bp_data_t* bp_data = NULL;
    int32_t pos = -1;
    int32_t vol = BP_VOL_MAX;

    for(int32_t index = 0; index < num; index++){
        if (!_impl(_this)->m_bp_is_enable[index]) continue;
        bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, bp_list[index]);

        if(vol > bp_data->m_vol){
            vol = bp_data->m_vol;
            pos = bp_list[index];
        }
    }
    return pos;
}
static void sm_charger_align_left_bp_list(sm_sv_charger_t *_this){
//    charger_print_waiting_list(_this);
    int8_t left_id = 0;
    int8_t right_id = BP_NUMBER_CHARGER_SUPPORT-1;
    while (left_id < right_id &&
            left_id < BP_NUMBER_CHARGER_SUPPORT &&
            right_id > -1){
        if (_impl(_this)->m_charging_bp_list[left_id] != -1)
        {
            left_id++;
            continue;
        }
        if (_impl(_this)->m_charging_bp_list[right_id] == -1)
        {
            right_id--;
            continue;
        }
        if (left_id < right_id) {
            if (_impl(_this)->m_charging_bp_list[left_id] == -1 &&
                _impl(_this)->m_charging_bp_list[right_id] != -1){
                _impl(_this)->m_charging_bp_list[left_id] = _impl(_this)->m_charging_bp_list[right_id];
                _impl(_this)->m_charging_bp_list[right_id] = -1;
                left_id++;
                right_id--;
            }
        }else
            break;
    }
    left_id = 0;
    right_id = BP_NUMBER_CHARGER_SUPPORT-1;
    while (left_id < right_id &&
            left_id < BP_NUMBER_CHARGER_SUPPORT &&
            right_id > -1){
        if (_impl(_this)->m_waiting_bp_list[left_id] != -1)
        {
            left_id++;
            continue;
        }
        if (_impl(_this)->m_waiting_bp_list[right_id] == -1)
        {
            right_id--;
            continue;
        }
        if (left_id < right_id) {
            if (_impl(_this)->m_waiting_bp_list[left_id] == -1 && _impl(_this)->m_waiting_bp_list[right_id] != -1){
                _impl(_this)->m_waiting_bp_list[left_id] = _impl(_this)->m_waiting_bp_list[right_id];
                _impl(_this)->m_waiting_bp_list[right_id] = -1;
                left_id++;
                right_id--;
            }
        }else
            break;
    }
}
static int32_t pos = -1;
static int32_t sm_charger_check_merging_condition(sm_sv_charger_t* _this, int32_t* _pos){
    const sm_bp_data_t* bp_data = NULL;
    int32_t vol_diff = 0;
    int32_t vol = BP_VOL_MAX;
    pos = -1;
    for (int i = 0; i < _impl(_this)->m_waiting_bp_number ; ++i) {
        if (_impl(_this)->m_waiting_bp_list[i] == -1 || !_impl(_this)->m_bp_is_enable[i]) continue;
        bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _impl(_this)->m_waiting_bp_list[i]);
        if(vol > bp_data->m_vol){
            vol = bp_data->m_vol;
            pos = _impl(_this)->m_waiting_bp_list[i];
        }
    }
    if (pos == -1 || _impl(_this)->m_bp_is_enable[pos] == 0) return SM_CHARGE_NULL;
    *_pos = pos;
    bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _impl(_this)->m_charging_bp_list[0]);
    vol_diff = (vol - bp_data->m_vol)*10;
    if (abs(vol_diff) < SM_SV_CHARGER_MERGE_DIFF_VOL)
    {
        return SM_CHARGE_MERGE;
    }
    else if (vol_diff < 0)
        return SM_CHARGE_SWITCH;
    else
        return SM_CHARGE_NULL;
}

static int32_t sm_charger_err_update(sm_sv_charger_t* _this){

}

static int32_t sm_charger_get_cur(sm_sv_charger_t* _this){
    const sm_bp_data_t* bp_data = NULL;
    int32_t cur = 0;
    for (int i = 0; i < _impl(_this)->m_charging_bp_number; ++i) {
        bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _impl(_this)->m_charging_bp_list[i]);
        cur += bp_data->m_cur;
        bp_data = NULL;
    }
    return cur;
}

static int32_t sm_charger_update_data(sm_sv_charger_t* _this){
    if (_impl(_this)->m_state == SM_CHARGED)
        _impl(_this)->m_data.m_state = SM_CHARGER_CHARGING;
    else if (_impl(_this)->m_state == SM_NOT_CHARGED ||
            _impl(_this)->m_state == SM_STOP_CHARGING)
        _impl(_this)->m_data.m_state = SM_CHARGER_NOT_CHARGING;

    _impl(_this)->m_data.m_vol  = _impl(_this)->m_if->get_charger_vol_fn_t();
    _impl(_this)->m_data.m_cur  = sm_charger_get_cur(_this);
    _impl(_this)->m_data.m_charging_bp_num = _impl(_this)->m_charging_bp_number;
    _impl(_this)->m_data.m_err = _impl(_this)->m_err;
//    _impl(_this)->m_event_cb->on_update_data(&_impl(_this)->m_data, _impl(_this)->m_event_arg);
}

static int32_t next_bp;
static int32_t bp_pos = -1;
static int32_t sm_charger_update_state(sm_sv_charger_t* _this){
    const sm_bp_data_t* bp_data = NULL;
    sm_charger_align_left_bp_list(_this);
    switch (_impl(_this)->m_state) {
        case SM_CHARGER_IDLE: {
            if (_impl(_this)->m_plugged_in == SM_CHARGER_PLUGGED_IN &&
                !elapsed_timer_get_remain(&_impl(_this)->m_state_timeout)) {
                _impl(_this)->m_state = SM_START_CHARGING;
            }
        }
            break;
        case SM_START_CHARGING: {
            bp_pos = sm_charger_find_first_bp(_this);
            if (bp_pos < 0) {
                _impl(_this)->m_state = SM_NOT_CHARGED;
                return -1;
            }
            _impl(_this)->m_waiting_bp = bp_pos;
            if (sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                                 bp_pos,
                                 BP_CMD_CHARGE,
                                 NULL,
                                 sm_charging_bp_cmd_handle,
                                 _this)) {
                _impl(_this)->m_state = SM_NOT_CHARGED;
                return -1;
            }
            elapsed_timer_reset(&_impl(_this)->m_state_timeout);
            _impl(_this)->m_state = SM_WAITING_START_CHARGING;
        }
            break;
        case SM_WAITING_START_CHARGING: {
//            LOG_DBG(TAG, "SM_WAITING_START_CHARGING");
            if (!elapsed_timer_get_remain(&_impl(_this)->m_state_timeout)) {
                _impl(_this)->m_state = SM_NOT_CHARGED;
                LOG_ERR(TAG, "WAITING START CHARGING ... TIMEOUT");
                return -1;
            }
            if (_impl(_this)->m_waiting_bp == -1) {
                _impl(_this)->m_state = SM_CHARGED;
                _impl(_this)->m_event_cb->on_charged(_impl(_this)->m_event_arg);
                elapsed_timer_reset(&_impl(_this)->m_switch_merge_timeout);
            }
        }
            break;
        case SM_CHARGED: {
//            LOG_DBG(TAG, "CHARGED");
#if SM_CHARGER_CHECK_CUR
            if (sm_charger_get_cur(_this) < 100){
                _impl(_this)->m_state = SM_STOP_CHARGING;
                break;
            }
#endif
            next_bp = -1;
            switch (sm_charger_check_merging_condition(_this, &next_bp)) {
                case SM_CHARGE_MERGE:
                    if (!elapsed_timer_get_remain(&_impl(_this)->m_switch_merge_timeout))
                    {
                        LOG_DBG(TAG, "MERGE WITH BP %d", next_bp);
                        _impl(_this)->m_state = SM_MERGE;
                    }

                    break;
                case SM_CHARGE_SWITCH:
                    if (!elapsed_timer_get_remain(&_impl(_this)->m_switch_merge_timeout))
                    {
                        LOG_DBG(TAG, "SWITCH TO BP %d", next_bp);
                        _impl(_this)->m_state = SM_SWITCH;
                    }

                    break;
                case SM_CHARGE_NULL:
                    break;
            }
            elapsed_timer_reset(&_impl(_this)->m_state_timeout);
        }
            break;
        case SM_MERGE: {
//            LOG_DBG(TAG, "MERGE");
            if (!elapsed_timer_get_remain(&_impl(_this)->m_state_timeout)) {
                _impl(_this)->m_state = SM_NOT_CHARGED;
                LOG_ERR(TAG, "MERGE ... TIMEOUT");
                return -1;
            }
            if (_impl(_this)->m_waiting_bp == -1) {
                sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                                 (int32_t) next_bp,
                                 BP_CMD_CHARGE,
                                 NULL,
                                 sm_charging_bp_cmd_handle,
                                 _this);
                _impl(_this)->m_waiting_bp = next_bp;
                _impl(_this)->m_state = SM_WAITING_START_CHARGING;
                elapsed_timer_reset(&_impl(_this)->m_state_timeout);
            }
        }
            break;
        case SM_SWITCH: {
//            LOG_DBG(TAG, "SWITCH");
            if (!elapsed_timer_get_remain(&_impl(_this)->m_state_timeout)) {
                _impl(_this)->m_state = SM_NOT_CHARGED;
                LOG_ERR(TAG, "SWITCH ... TIMEOUT");
                return -1;
            }
            if (_impl(_this)->m_waiting_bp != -1)
                return 0;
            if (_impl(_this)->m_charging_bp_number != 0) {
                sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                                 _impl(_this)->m_charging_bp_list[0],
                                 BP_CMD_STANDBY,
                                 NULL,
                                 sm_charging_bp_cmd_handle,
                                 _this);
                _impl(_this)->m_waiting_bp = _impl(_this)->m_charging_bp_list[0];
                elapsed_timer_reset(&_impl(_this)->m_state_timeout);
                return 0;
            }
            if (_impl(_this)->m_charging_bp_number == 0 && _impl(_this)->m_waiting_bp == -1) {
                sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                                 next_bp,
                                 BP_CMD_CHARGE,
                                 NULL,
                                 sm_charging_bp_cmd_handle,
                                 _this);
                _impl(_this)->m_waiting_bp = next_bp;
                _impl(_this)->m_state = SM_WAITING_START_CHARGING;
                elapsed_timer_reset(&_impl(_this)->m_state_timeout);
            }
        }
            break;

        case SM_PRE_FORCED:
        case SM_STOP_CHARGING: {
            if (_impl(_this)->m_waiting_bp != -1)
                break;
            if (_impl(_this)->m_charging_bp_number != 0) {
                _impl(_this)->m_waiting_bp = _impl(_this)->m_charging_bp_list[0];
                if (_impl(_this)->m_waiting_bp != -1) {
                    sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                                     _impl(_this)->m_waiting_bp,
                                     BP_CMD_STANDBY,
                                     NULL,
                                     sm_charging_bp_cmd_handle,
                                     _this);
                    break;
                }
            }
            if (_impl(_this)->m_forced_bp != -1 &&
                _impl(_this)->m_charging_bp_number == 0 &&
                _impl(_this)->m_waiting_bp == -1 &&
                _impl(_this)->m_plugged_in == 1) {
                LOG_DBG(TAG, "FORCED TO BP %d", _impl(_this)->m_forced_bp);
                _impl(_this)->m_state = SM_FORCED;
            }
        }
            break;
        case SM_NOT_CHARGED:{
            _impl(_this)->m_state = SM_CHARGER_IDLE;
            _impl(_this)->m_waiting_bp = -1;
            _impl(_this)->m_forced_bp = -1;
            elapsed_timer_resetz(&_impl(_this)->m_state_timeout, CHARGER_IDLE_TIMEOUT);
        }
            break;
        case SM_FORCED:{
            if (!elapsed_timer_get_remain(&_impl(_this)->m_state_timeout)) {
                _impl(_this)->m_forced_bp = -1;
                _impl(_this)->m_state = SM_STOP_CHARGING;
                LOG_ERR(TAG, "FORCE ... TIMEOUT");
                return -1;
            }

            if (_impl(_this)->m_waiting_bp != -1)
                break;

            if (_impl(_this)->m_charging_bp_number == 1 &&
                _impl(_this)->m_charging_bp_list[0] == _impl(_this)->m_forced_bp){
                elapsed_timer_resetz(&_impl(_this)->m_state_timeout, CHARGER_FORCE_TIMEOUT);
                break;
            }

            bp_data = sm_sv_bp_get_data(_impl(_this)->m_bpm, _impl(_this)->m_forced_bp);
            if (bp_data->m_state == BP_STATE_STANDBY){
                _impl(_this)->m_waiting_bp = _impl(_this)->m_forced_bp;
                sm_sv_bp_set_cmd(_impl(_this)->m_bpm,
                                 _impl(_this)->m_waiting_bp,
                                 BP_CMD_CHARGE,
                                 NULL,
                                 sm_charging_bp_cmd_handle,
                                 _this);
                bp_data = NULL;
                elapsed_timer_resetz(&_impl(_this)->m_state_timeout, CHARGER_FORCE_TIMEOUT);
            }
#if SM_CHARGER_CHECK_CUR
            if (sm_charger_get_cur(_this) < 100)
                _impl(_this)->m_state = SM_STOP_CHARGING;
#endif
        }
            break;
        default:
            break;
    }
    return 0;
}

int32_t sm_sv_charger_process(sm_sv_charger_t* _this){
    if (!_this){
        return -1;
    }

    if (is_charger_plugged_in(_this) && _impl(_this)->m_plugged_in == SM_CHARGER_UNPLUGGED_IN) {
        _impl(_this)->m_plugged_in = SM_CHARGER_PLUGGED_IN;
        if (_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_plugged_in) {
            _impl(_this)->m_event_cb->on_plugged_in(SM_CHARGER_PLUGGED_IN, _impl(_this)->m_event_arg);
        }
        _impl(_this)->m_state = SM_START_CHARGING;
    } else if (!is_charger_plugged_in(_this) && _impl(_this)->m_plugged_in == SM_CHARGER_PLUGGED_IN) {
        _impl(_this)->m_plugged_in = SM_CHARGER_UNPLUGGED_IN;
        if (_impl(_this)->m_event_cb && _impl(_this)->m_event_cb->on_plugged_in) {
            _impl(_this)->m_event_cb->on_plugged_in(SM_CHARGER_UNPLUGGED_IN, _impl(_this)->m_event_arg);
        }
        _impl(_this)->m_forced_bp = -1;
        _impl(_this)->m_state = SM_STOP_CHARGING;
    }

    sm_charger_update_state(_this);
    sm_charger_update_data(_this);
//    sm_charger_err_update(_this);
    return 0;
}
/*
void charger_print_waiting_list(sm_sv_charger_t *_this){
    printf("Waiting list %d : ", _impl(_this)->m_waiting_bp_number);
    for (int i=0; i<3; i++){
        printf("%d ", _impl(_this)->m_waiting_bp_list[i]);
    }
    printf("\n");
}
void charger_print_charging_list(sm_sv_charger_t *_this){
    printf("Charging list %d : ", _impl(_this)->m_charging_bp_number);
    for (int i=0; i<3; i++){
        printf("%d ", _impl(_this)->m_charging_bp_list[i]);
    }
    printf("\n");
}

void inc_vol_diff_500mV(){
    SM_SV_CHARGER_MERGE_DIFF_VOL += 500;
    LOG_DBG(TAG, "SM_SV_CHARGER_MERGE_DIFF_VOL = %d", SM_SV_CHARGER_MERGE_DIFF_VOL);
}
void dec_vol_diff_500mV(){
    SM_SV_CHARGER_MERGE_DIFF_VOL -= 500;
    if (SM_SV_CHARGER_MERGE_DIFF_VOL <= 0 ) SM_SV_CHARGER_MERGE_DIFF_VOL = 0;
    LOG_DBG(TAG, "SM_SV_CHARGER_MERGE_DIFF_VOL = %d", SM_SV_CHARGER_MERGE_DIFF_VOL);
}*/
#endif //EV_SDK_SM_SV_CHARGER_C_H
