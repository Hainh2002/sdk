//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_BT_H
#define EV_SDK_SM_SV_BT_H

#include <stdint.h>
#include <stdbool.h>

#define SM_SV_BT_HOLDING_TIME				x	/*ms*/
#define SM_SV_BT_TAPPING_TIME				x	/*ms*/
#define SM_SV_BT_DOUBLE_TAPPING_TIME		x	/*ms*/

#define SM_SV_BT_MAX_BT_NUM			10

typedef enum SM_SV_BT_EVENT {
	SM_SV_BT_EVENT_TAP,
	SM_SV_BT_EVENT_HOLD,
	SM_SV_BT_EVENT_DOUBLE_TAP
} SM_SV_BT_EVENT;

typedef enum SM_SV_BT_STATE {
	SM_SV_BT_STATE_RELEASED = 0,
	SM_SV_BT_STATE_PRESSED,
} SM_SV_BT_STATE;

typedef void sm_sv_bt_t;

typedef struct sm_sv_bt_config {
	uint32_t m_holding_time;
	uint32_t m_tapping_time;
	uint32_t m_double_tapping_time;
 uint32_t m_margin_time;
} sm_sv_bt_config_t;

typedef void (*sm_sv_bt_event_callback_fn_t)(uint8_t _btn_id, uint8_t _event, void *_arg);

typedef uint8_t (*sm_bt_if)();
// sm_bt_if g_bt_if[SM_SV_BT_MAX_BT_NUM];

/**
 * @brief sm_sv_bt_create
 * @return
 */
sm_sv_bt_t* sm_sv_bt_create(sm_bt_if _if[], uint8_t _button_num);
/**
 * @brief sm_sv_bt_destroy
 * @param _this
 * @return
 */
int32_t sm_sv_bt_destroy(sm_sv_bt_t *_this);
/**
 * @brief sm_sv_bt_destroy
 * @param _this
 * @param _fn_callback
 * @param _arg
 * @return
 */
int32_t sm_sv_bt_reg_event(sm_sv_bt_t *_this,uint8_t _button_id, sm_sv_bt_event_callback_fn_t _fn_callback, void *_arg);
// /**
//  * @brief sm_sv_bt_set_if
//  * @param _this
//  * @param _bt_if
//  */
// void sm_sv_bt_set_if(sm_sv_bt_t *_this, sm_sv_bt_if_t *_bt_if);
/**
 * @brief sm_sv_bt_set_config
 * @param _this
 * @param _bt_config
 */
void sm_sv_bt_set_common_config(sm_sv_bt_t *_this, sm_sv_bt_config_t *_bt_config);
/**
 * @brief sm_sv_bt_set_config
 * @param _this
 * @param _bt_config
 */
void sm_sv_bt_set_config(sm_sv_bt_t *_this, uint8_t _button_id, sm_sv_bt_config_t *_bt_config);
/**
 * @brief sm_sv_bt_get_config
 * @param _this
 * @param _bt_config
 */
void sm_sv_bt_get_config(sm_sv_bt_t *_this, bool _is_common, uint8_t _button_id, sm_sv_bt_config_t *_bt_config);
// /**
//  * @brief sm_sv_bt_get_input
//  * @param _this
//  * @param _bt_if
//  * @param _index
//  * @return
//  */
// uint8_t sm_sv_bt_get_status(sm_sv_bt_t *_this, uint8_t *_bt_if[], uint8_t _index);

void sm_sv_bt_process(sm_sv_bt_t *_this);

#endif //EV_SDK_SM_SV_BT_H
