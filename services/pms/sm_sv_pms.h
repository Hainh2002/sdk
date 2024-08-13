//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_PMS_H
#define EV_SDK_SM_SV_PMS_H


#include <stdint.h>
#include <stdlib.h>
#include "sm_sv_bp.h"
#include "sm_pms_est_data.h"
#include "sm_pms_data.h"

#define SM_SV_PMS_EVENT_HANDLER_NUMBER							10

typedef enum SM_SV_PMS_EVENT {
	SM_SV_PMS_EVENT_IDLE = 0,
	SM_SV_PMS_EVENT_SWITCH_SUCCESS,
	SM_SV_PMS_EVENT_SWITCH_FAIL,
	SM_SV_PMS_EVENT_MERGE_SUCCESS,
	SM_SV_PMS_EVENT_MERGE_FAIL,
} SM_SV_PMS_EVENT;

typedef void sm_sv_pms_t;

typedef struct sm_sv_pms_cb {
    void (*switch_merge_cb_fn)(uint8_t , uint8_t , uint8_t , void*);
    void (*update_data_cb_fn)(const sm_pms_data_t*, void*);
    void (*update_est_data_cb_fn)(const est_data_t* , void*);
}sm_sv_pms_event_cb_fn_t;



/**
 * @brief sm_sv_pms_create
 * @param _bp_num
 * @param _bp_if : node id control
 * @param _charger_if: charger mos-fet control
 * @return
 */
sm_sv_pms_t* sm_sv_pms_create(sm_sv_bp_t *_bp);
/**
 * @brief sm_sv_pms_destroy
 * @param _this
 * @return
 */
int32_t sm_sv_pms_destroy(sm_sv_pms_t* _this);
/**
 * @brief sm_sv_pms_set_config
 * @param _this
 * @param _pms_config
 * @return
 */
int32_t sm_sv_pms_set_config(sm_sv_pms_t* _this,
							sm_sv_pms_config_t* _pms_config);
/**
 * @brief sm_sv_pms_get_config
 * @param _this
 * @param _pms_config
 * @return
 */
int32_t sm_sv_pms_get_config(sm_sv_pms_t* _this,
							sm_sv_pms_config_t* _pms_config);
/**
 * @brief sm_sv_pms_reg_event
 * @param _this
 * @param _fn_cb
 * @param _arg
 * @return
 */
int32_t sm_sv_pms_reg_event(sm_sv_pms_t* _this,
							sm_sv_pms_event_cb_fn_t *_cb,
							void* _arg);
/**
 * @brief sm_sv_pms_switch_merge_enable
 * @param _this
 * @param _port : 1,2,3
 * @param _is_enable : 0,1
 * @return
 */
int32_t sm_sv_pms_switch_merge_enable(sm_sv_pms_t* _this, uint8_t _port, uint8_t _is_enable);

/**
 * @brief sm_sv_pms_get_active_port
 * @param _this
 * @param _active_port
 * @return size of string of active port
 */
int32_t sm_sv_pms_get_active_port(sm_sv_pms_t* _this, int8_t* _active_port[]);
/**
 * @brief sm_sv_pms_get_inactive_port
 * @param _this
 * @param _inactive_port
 * @return
 */
int32_t sm_sv_pms_get_inactive_port(sm_sv_pms_t* _this, int8_t* _inactive_port[]);
/**
 * @brief sm_sv_pms_force_switch
 * @param _this
 * @param _forced_bp_id
 * @return
 */
int32_t sm_sv_pms_force_switch(sm_sv_pms_t* _this, int8_t _forced_bp_id);
/**
 * @brief sm_sv_pms_force_merge
 * @param _this
 * @param _forced_bp_id
 * @return
 */
int32_t sm_sv_pms_force_merge(sm_sv_pms_t* _this, int8_t _forced_bp_id);
/**
 * @brief sm_sv_pms_release_switch
 * @param _this
 * @param _released_bp_id
 * @param _next_working_bp_id
 * @return
 */
int32_t sm_sv_pms_release_switch(sm_sv_pms_t* _this, int8_t _released_bp_id);
/**
 * @brief sm_sv_pms_disable_invalid_bps
 * @param _this
 * @return
 */
int32_t sm_sv_pms_disable_invalid_bps(sm_sv_pms_t* _this);
/**
 * @brief sm_sv_pms_process
 * @param _this
 * @return
 */
int32_t sm_sv_pms_process(sm_sv_pms_t* _this);
/**
 * @brief sm_sv_pms_estimate
 * @param _this
 * @return
 */
int32_t sm_sv_pms_get_estimate_data(sm_sv_pms_t* _this, est_data_t* _est_data);

/**
 *
 * @param _pms
 * @param _new_data
 * @return
 */
int32_t sm_sv_pms_receive_data(sm_sv_pms_t * _pms, sm_pms_data_t *_new_data);

void pms_bp_connected_cb_fn(int32_t _id, const char* _sn, int32_t _soc, void* _arg);
void pms_bp_disconnected_cb_fn(int32_t _id, const char* _sn, void* _arg);
void pms_bp_update_data_cb_fn(int32_t _id, const sm_bp_data_t* _data, void* _arg);


#endif //EV_SDK_SM_SV_PMS_H
