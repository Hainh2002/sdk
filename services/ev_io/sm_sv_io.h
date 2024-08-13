//
// Created by vnbk on 27/02/2024.
//

#ifndef EV_SDK_SM_SV_IO_H
#define EV_SDK_SM_SV_IO_H

#define SM_SV_IO_MAX_IO_NUM             (uint8_t) 48
#define SM_SV_IO_BLINK_DUR_MS           500
#define SM_SV_IO_BLINK_PER_MS           1000

#include <stdint.h>
#include <stdlib.h>

//#include "sm_sv_bt.h"

typedef enum SM_SV_IO_EVENT {
	SM_SV_IO_EVENT_IN_KEY_CHANGED_ON,
	SM_SV_IO_EVENT_IN_KEY_CHANGED_OFF,

	SM_SV_IO_EVENT_IN_REVERSE_CHANGED_ON,
	SM_SV_IO_EVENT_IN_REVERSE_CHANGED_OFF,

	SM_SV_IO_EVENT_IN_LEFT_CHANGED_ON,
	SM_SV_IO_EVENT_IN_LEFT_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_LEFT_CHANGED_ON,
	SM_SV_IO_EVENT_OUT_LEFT_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_LEFT_CHANGED_BLINK,

	SM_SV_IO_EVENT_IN_RIGHT_CHANGED_ON,
	SM_SV_IO_EVENT_IN_RIGHT_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_ON,
	SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_BLINK,

	SM_SV_IO_EVENT_IN_PHASE_CHANGED_ON,
	SM_SV_IO_EVENT_IN_PHASE_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_PHASE_CHANGED_ON,
	SM_SV_IO_EVENT_OUT_PHASE_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_PHASE_CHANGED_BLINK,

	SM_SV_IO_EVENT_IN_HORN_CHANGED_ON,
	SM_SV_IO_EVENT_IN_HORN_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_HORN_CHANGED_ON,
	SM_SV_IO_EVENT_OUT_HORN_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_HORN_CHANGED_BLINK,

	SM_SV_IO_EVENT_IN_COS_CHANGED_ON,
	SM_SV_IO_EVENT_IN_COS_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_COS_CHANGED_ON,
	SM_SV_IO_EVENT_OUT_COS_CHANGED_OFF,
	SM_SV_IO_EVENT_OUT_COS_CHANGED_BLINK,

	SM_SV_IO_EVENT_IN_BREAK_CHANGED_ON,
	SM_SV_IO_EVENT_IN_BREAK_CHANGED_OFF,

	SM_SV_IO_EVENT_OUT_TAIL_LIGHT_CHANGED_ON,
	SM_SV_IO_EVENT_OUT_TAIL_LIGHT_CHANGED_OFF,
    SM_SV_IO_EVENT_IN_PARKING_PRESSED,
    SM_SV_IO_EVENT_IN_DRV_MODE_PRESSED,
} SM_SV_IO_EVENT;

typedef void (*sm_sv_io_event_cb_fn_t)(uint8_t _event, void* _arg);

typedef struct sm_sv_io_if {
	uint8_t (*in_key_fn_t)();
	uint8_t (*in_left_light_fn_t)();
	uint8_t (*in_right_light_fn_t)();
	uint8_t (*in_phase_light_fn_t)();
	uint8_t (*in_cos_light_fn_t)();
	uint8_t (*in_parking_fn_t)();
	uint8_t (*in_driving_mode_fn_t)();
	uint8_t (*in_left_break_fn_t)();
	uint8_t (*in_right_break_fn_t)();
	uint8_t (*in_reverse_fn_t)();
    uint8_t (*in_horn_fn_t)();

	void (*out_left_light_fn_t)(uint8_t _value);
	void (*out_right_light_fn_t)(uint8_t _value);
	void (*out_phase_light_fn_t)(uint8_t _value);
	void (*out_code_light_fn_t)(uint8_t _value);
	void (*out_tail_light_fn_t)(uint8_t _value);
	void (*out_horn_fn_t)(uint8_t _value);
} sm_sv_io_if_t;

typedef void sm_sv_io_t;

/**
 * @brief sm_sv_io_create
 * @param _bt
 * @return
 */
sm_sv_io_t* sm_sv_io_create(void *_if);
/**
 * @brief sm_sv_io_destroy
 * @param _this
 * @return
 */
int32_t sm_sv_io_destroy(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_reg_event
 * @param _this
 * @param _fn_cb
 * @param _arg
 * @return
 */
int32_t sm_sv_io_reg_event(sm_sv_io_t *_this, sm_sv_io_event_cb_fn_t _fn_cb, void* _arg);
/**
 * @brief sm_sv_io_set_if
 * @param _this
 * @param _if
 * @return
 */
int32_t sm_sv_io_set_if(sm_sv_io_t *_this, sm_sv_io_if_t *_io_if);

/**
 * @brief sm_sv_io_get_input_key
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_key(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_left_light
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_left_light(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_right_light
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_right_light(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_phare_light
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_phare_light(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_parking
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_parking(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_driving_mode
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_driving_mode(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_left_break
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_left_break(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_right_break
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_right_break(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_reverse
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_reverse(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_get_input_horn
 * @param _this
 * @return
 */
uint8_t sm_sv_io_get_input_horn(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_set_output_left_light
 * @param _this
 * @param _value
 */
void sm_sv_io_set_output_left_light(sm_sv_io_t *_this, uint8_t _value);
/**
 * @brief sm_sv_io_release_output_left_light
 * @param _this
 */
void sm_sv_io_release_output_left_light(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_set_output_right_light
 * @param _this
 * @param _value
 */
void sm_sv_io_set_output_right_light(sm_sv_io_t *_this, uint8_t _value);
/**
 * @brief sm_sv_io_release_output_right_light
 * @param _this
 */
void sm_sv_io_release_output_right_light(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_set_output_phare_light
 * @param _this
 * @param _value
 */
void sm_sv_io_set_output_phare_light(sm_sv_io_t *_this, uint8_t _value);
/**
 * @brief sm_sv_io_release_output_phare_light
 * @param _this
 */
void sm_sv_io_release_output_phare_light(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_set_output_horn_light
 * @param _this
 * @param _value
 */
void sm_sv_io_set_output_horn(sm_sv_io_t *_this, uint8_t _value);
/**
 * @brief sm_sv_io_release_output_horn
 * @param _this
 */
void sm_sv_io_release_output_horn(sm_sv_io_t *_this);
/**
 * @brief sm_sv_io_set_output_tail_light
 * @param _this
 * @param _value
 */
void sm_sv_io_set_output_tail_light(sm_sv_io_t *_this, uint8_t _value);
/**
 * @brief sm_sv_io_release_output_tail_light
 * @param _this
 * @param _value
 */
void sm_sv_io_release_output_tail_light(sm_sv_io_t *_this, uint8_t _value);
/**
* @brief sm_sv_io_set_output_code_light
* @param _this
* @param _value
*/
void sm_sv_io_set_output_code_light(sm_sv_io_t *_this, uint8_t _value);
/**
* @brief sm_sv_io_release_output_code_light
* @param _this
* @param _value
*/
void sm_sv_io_release_output_code_light(sm_sv_io_t *_this);

/**
 * @brief sm_sv_io_set_left_light_blink
 * @param _this
 * @param _duration
 * @param _period
 */
void sm_sv_io_set_left_light_blink(sm_sv_io_t *_this, uint32_t _duration, uint32_t _period);
/**
 * @brief sm_sv_io_release_output_left_light
 * @param _this
 */
void sm_sv_io_release_left_light_blink(sm_sv_io_t *_this);

/**
 * @brief sm_sv_io_set_right_light_blink
 * @param _this
 * @param _duration
 * @param _period
 */
void sm_sv_io_set_right_light_blink(sm_sv_io_t *_this, uint32_t _duration, uint32_t _period);
/**
 * @brief sm_sv_io_release_output_left_light
 * @param _this
 */
void sm_sv_io_release_right_light_blink(sm_sv_io_t *_this);

/**
 * @brief sm_sv_io_set_horn_blink
 * @param _this
 * @param _duration
 * @param _period
 */
void sm_sv_io_set_horn_blink(sm_sv_io_t *_this, uint32_t _duration, uint32_t _period);
/**
 * @brief sm_sv_io_release_output_left_light
 * @param _this
 */
void sm_sv_io_release_output_horn_blink(sm_sv_io_t *_this);
/**
 *
 * @param _this
 */
void sm_sv_io_process(sm_sv_io_t *_this);

#endif //EV_SDK_SM_SV_IO_H
