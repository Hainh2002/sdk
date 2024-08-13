#ifndef SM_BP_AUTH_H
#define SM_BP_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sm_bp.h"
#include "sm_bp_co.h"
#include "sm_co_if.h"

/**
 * @brief
 */
typedef void sm_bp_auth_t;

/**
 * @brief
 */
typedef struct{
    int32_t (*sm_bp_node_id_select)(int32_t);
    int32_t (*sm_bp_node_id_deselect)(int32_t);
}sm_bp_node_id_controller_t;

/**
 * @brief
 * @param _co_if
 * @param _bp_co
 * @param _node_id_ctl
 * @param _event_cb
 * @param _arg
 * @return
 */
sm_bp_auth_t* sm_bp_auth_create(sm_co_if_t* _co_if,
                                sm_bp_co_t* _bp_co,
                                sm_bp_node_id_controller_t * _node_id_ctl,
                                sm_bp_auth_event_fn_t _event_cb,
                                void* _arg);
/**
 * @brief
 * @param _this
 * @return
 */
int32_t sm_bp_auth_destroy(sm_bp_auth_t* _this);

/**
 * @brief
 * @param _this
 * @param _bp
 * @return
 */
int32_t sm_bp_auth_start_auth(sm_bp_auth_t* _this, const sm_bp_t* _bp);

/**
 * @brief
 * @param _this
 * @return
 */
int32_t sm_bp_auth_get_bp_authenticating(sm_bp_auth_t* _this);

/**
 * @brief
 * @param _this
 * @return
 */
int32_t sm_bp_auth_process(sm_bp_auth_t* _this);

#ifdef __cplusplus
}
#endif

#endif