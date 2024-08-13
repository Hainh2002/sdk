//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_CHARGER_H
#define EV_SDK_SM_SV_CHARGER_H

#include <stdint.h>
#include "sm_sv_bp.h"

#define SM_CHARGER_CHECK_CUR            (0)
#define SM_SV_CHARGER_MAX_VOL           70000   // mV
#define SM_SV_CHARGER_MIN_VOL           54000   // mV
#define SM_SV_CHARGER_MAX_CUR           12000   // mA
#define SM_SV_CHARGER_STB_TIME          5000    // ms
#define SM_SV_CHARGER_MAX_TEMP          70      // oC
static uint32_t SM_SV_CHARGER_MERGE_DIFF_VOL   =   2000    ; // mV
typedef enum {
    SM_CHARGER_OVER_CUR,
    SM_CHARGER_OVER_VOL,
    SM_CHARGER_NO_CUR,
    SM_CHARGER_NO_VOL,
    SM_CHARGER_UNKNOWN,
    SM_CHARGER_ERR_NUMBER,
}SM_SV_CHARGER_ERR;

typedef enum {
    SM_CHARGER_NOT_CHARGING,
    SM_CHARGER_CHARGING,
    SM_CHARGER_ERR,
    SM_CHARGER_STATE_NUMBER
}SM_SV_CHARGER_STATE;

typedef struct {
    uint8_t m_charging_bp_num;
    int32_t* m_charging_bp_pos;
    int32_t m_vol;
    int32_t m_cur;
    int32_t m_err;
    SM_SV_CHARGER_STATE m_state;
}sm_sv_charger_data_t;

#define SM_CHARGER_PLUGGED_IN       (1)
#define SM_CHARGER_UNPLUGGED_IN     (0)

typedef struct {
    void (*on_plugged_in)(uint8_t, void*);
    void (*on_charged)(void*);
    void (*on_update_data)(sm_sv_charger_data_t*, void*);
    void (*on_error)(SM_SV_CHARGER_ERR, void*);
}sm_sv_charger_event_cb_fn_t;

typedef struct {
    uint32_t 	m_max_volt;
    uint32_t	m_min_volt;
    uint32_t	m_max_cur;
    uint32_t	m_power_stable_time;
    uint16_t	m_max_temp;
} sm_sv_charger_prof_t;

typedef void sm_sv_charger_t;

typedef struct {
    int32_t (*get_charger_vol_fn_t)();
}sm_sv_charger_if_t;

/**
 *
 * @param _if
 * @param _bpm
 * @return
 */
sm_sv_charger_t* sm_sv_charger_create_default(sm_sv_charger_prof_t* _prof, sm_sv_charger_if_t* _if, sm_sv_bp_t* _bpm);

/**
 * @brief:
 * @param _bp
 * @param _if
 * @return
 */
sm_sv_charger_t* sm_sv_charger_create(sm_sv_charger_prof_t* _prof, sm_sv_charger_if_t* _if, sm_sv_bp_t* _bp);

/**
 * @brief sm_sv_charger_destroy(sm_sv_charger_t* _this)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_destroy(sm_sv_charger_t* _this);

/**
 * @brief sm_sv_charger_reg_event(sm_sv_charger_t* _this, sm_sv_charger_event_callback_fn_t _fn_callback, void* _arg)
 * @param sm_sv_charger_t* _this
 * @param sm_sv_charger_event_callback_fn_t _fn_callback
 * @return
 * */
int32_t sm_sv_charger_reg_event(sm_sv_charger_t* _this, const sm_sv_charger_event_cb_fn_t* _fn_cb, void* _arg);

/**
 * @brief
 * @param _this
 * @param _prof
 * @return
 */
int32_t sm_sv_charger_set_profile(sm_sv_charger_t* _this, const sm_sv_charger_prof_t* _prof);

/**
 *
 * @param _this
 * @return
 */
const sm_sv_charger_prof_t* sm_sv_charger_get_profile(sm_sv_charger_t* _this);

/**
 * @brief
 * @param _this
 * @return
 */
const sm_sv_charger_data_t* sm_sv_charger_get_data(sm_sv_charger_t* _this);

/**
 * @brief sm_sv_charger_get_bp_num(sm_sv_charger_t* _this)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_get_bp_num(sm_sv_charger_t* _this);

/**
 * @brief sm_sv_charger_get_cur(sm_sv_charger_t* _this)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_get_cur(sm_sv_charger_t* _this);

/**
 * @brief sm_sv_charger_get_volt(sm_sv_charger_t* _this)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_get_volt(sm_sv_charger_t* _this);

/**
 * @brief sm_sv_charger_get_state(sm_sv_charger_t* _this)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_get_state(sm_sv_charger_t* _this);

/**
 * @brief sm_sv_charger_set_charge(sm_sv_charger_t* _this, uint8_t _bp)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_force(sm_sv_charger_t* _this, uint8_t _bp_id);

/**
 * @brief sm_sv_charger_set_discharge(sm_sv_charger_t* _this, uint8_t _bp)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_release(sm_sv_charger_t* _this);

int32_t sm_sv_charger_disable_bp(sm_sv_charger_t* _this, uint8_t _bp_id);
int32_t sm_sv_charger_enable_bp(sm_sv_charger_t* _this, uint8_t _bp_id);
/**
 * @brief sm_sv_charger_process(sm_sv_charger_t* _this)
 *
 * @param sm_sv_charger_t* _this
 *
 * */
int32_t sm_sv_charger_process(sm_sv_charger_t* _this);
void sm_sv_charger_on_bp_connected(int32_t _id, const char* _sn, int32_t _soc, void* _arg);
void sm_sv_charger_on_bp_disconnected(int32_t _id, const char* _sn, void* _arg);

/*void charger_print_charging_list(sm_sv_charger_t *_this);
void charger_print_waiting_list(sm_sv_charger_t *_this);
void inc_vol_diff_500mV();
void dec_vol_diff_500mV();*/
#endif //EV_SDK_SM_SV_CHARGER_H
