#include <stdio.h>

#include "sm_bp_auth.h"
#include "sm_logger.h"

#define TAG "SM_BP_AUTH"

#define BP_ASSIGNING_TIMEOUT                        500     ///3000
#define BP_ASSIGNING_WAIT_SLAVE_DESELECT_TIMEOUT    4000
#define BP_ASSIGNING_AUTH_TIMEOUT                   6000    //3000
#define BP_ASSIGNING_RETRY_MAX                      2

#define CAN_NODE_ID_ASSIGN_COBID	                0x70
#define CAN_NODE_ID_OFFSET                          5

#define _impl(p)        ((sm_bp_auth_impl_t*)(p))

typedef enum {
    BP_ASSIGN_IDLE,
    BP_ASSIGN_WAIT_REQUEST,
    BP_ASSIGN_START,
    BP_ASSIGN_SLAVE_SELECT,
    BP_ASSIGN_SLAVE_SELECT_CONFIRM,
    BP_ASSIGN_WAIT_CONFIRM,
    BP_ASSIGN_AUTHORIZING_START,
    BP_ASSIGN_AUTHORIZING,
    BP_ASSIGN_VERIFY_AUTH,
    BP_ASSIGNED,
    BP_ASSIGN_FAILURE,
    BP_ASSIGN_WAIT_SLAVE_DESELECT
}BP_ASSIGN_STATE;

typedef struct {
    sm_bp_t* m_assigning_bp;
    BP_ASSIGN_STATE m_assigning_state;
    elapsed_timer_t m_assigning_timeout;

    sm_co_if_t* m_co_if;

    sm_bp_node_id_controller_t* m_node_id_ctl;
    sm_bp_co_t* m_bp_co;

    sm_bp_cmd_t m_auth_cmd;
    char m_auth_data[BP_DEVICE_SN_SIZE];

    sm_bp_auth_event_fn_t m_cb;
    void* m_cb_arg;
}sm_bp_auth_impl_t;

static sm_bp_auth_impl_t g_bp_auth;

static void sm_bp_auth_reset(sm_bp_auth_t* _this) {
    _impl(_this)->m_assigning_bp = NULL;
    _impl(_this)->m_assigning_state = BP_STATE_IDLE;
    _impl(_this)->m_auth_cmd.m_id = -1;
    elapsed_timer_resetz(&_impl(_this)->m_assigning_timeout, BP_ASSIGNING_TIMEOUT);
}

static uint8_t sm_bp_auth_verify_sn(const char* _src, char* _dest){
    int16_t index = 0;
    int16_t dest_index = 0;
    char value = _src[index];

    while(value != '\0'){
        value = _src[index];

        if(value >= '0' && value <= '9') {
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value >= 'A' && value <= 'Z'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value >= 'a' && value <= 'z'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value == '.'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        if(value == '_'){
            _dest[dest_index++] = value;
            index++;
            continue;
        }
        return 0;
    }
    return 1;
}

static void sm_bp_on_auth_cmd(int32_t _id, SM_BP_CMD _cmd, int32_t _success, void* _data, void* _arg){
    sm_bp_auth_impl_t* bp_auth = (sm_bp_auth_impl_t*)(_arg);
    if(!bp_auth || !bp_auth->m_assigning_bp){
        return;
    }
    if(_id == bp_auth->m_assigning_bp->m_id && _cmd == BP_CMD_READ_SN){
        if(_success == SM_BP_CMD_SUCCESS){
            bp_auth->m_assigning_state = BP_ASSIGN_VERIFY_AUTH;
        }else{
            bp_auth->m_assigning_state = BP_ASSIGN_FAILURE;
        }
    }
}

static void sm_bp_auth_recv_data(const uint32_t _can_id, uint8_t* _data, void* _arg){
    sm_bp_auth_impl_t* bp_auth = (sm_bp_auth_impl_t*)_arg;
    if(!_arg){
        return;
    }
    if (_can_id == CAN_NODE_ID_ASSIGN_COBID) {
        if (!bp_auth->m_assigning_bp) {
            LOG_ERR(TAG, "No BP is assigning");
            return;
        }
        if (bp_auth->m_assigning_state == BP_ASSIGN_WAIT_REQUEST) {
            bp_auth->m_assigning_state = BP_ASSIGN_START;

            LOG_DBG(TAG, "Assigning state is switched from BP_ASSIGN_WAIT_REQUEST to BP_ASSIGN_START");
        } else if (bp_auth->m_assigning_state == BP_ASSIGN_SLAVE_SELECT) {
            bp_auth->m_assigning_state = BP_ASSIGN_SLAVE_SELECT_CONFIRM;
            LOG_DBG(TAG,
                    "Assigning state is switched from BP_ASSIGN_SLAVE_SELECT to BP_ASSIGN_SLAVE_SELECT_CONFIRM: %d",
                    bp_auth->m_assigning_bp->m_id);
        } else if (bp_auth->m_assigning_state == BP_ASSIGN_WAIT_CONFIRM) {
            if (_data[0]
                != (bp_auth->m_assigning_bp->m_id + CAN_NODE_ID_OFFSET)) {
                return;
            }

            bp_auth->m_assigning_state = BP_ASSIGN_AUTHORIZING_START;
            elapsed_timer_resetz(&bp_auth->m_assigning_timeout, BP_ASSIGNING_AUTH_TIMEOUT);

            LOG_DBG(TAG,
                    "Assigning state is switched from BP_ASSIGN_WAIT_CONFIRM to BP_ASSIGN_AUTHORIZING_START: %d",
                    bp_auth->m_assigning_bp->m_id);
        }
    }
}


sm_bp_auth_t* sm_bp_auth_create(sm_co_if_t* _co_if,
                                sm_bp_co_t* _bp_co,
                                sm_bp_node_id_controller_t * _node_id_ctl,
                                sm_bp_auth_event_fn_t _event_cb,
								void* _arg){
    if(!_node_id_ctl || !_bp_co || !_co_if){
        return NULL;
    }

    g_bp_auth.m_co_if = _co_if;

    g_bp_auth.m_bp_co = _bp_co;

    g_bp_auth.m_assigning_bp = NULL;
    g_bp_auth.m_assigning_state = BP_ASSIGN_IDLE;
    g_bp_auth.m_node_id_ctl = _node_id_ctl;
    g_bp_auth.m_cb = _event_cb;
    g_bp_auth.m_cb_arg = _arg;

    g_bp_auth.m_auth_cmd.m_id = -1;
    g_bp_auth.m_auth_cmd.m_cb = sm_bp_on_auth_cmd;
    g_bp_auth.m_auth_cmd.m_data = g_bp_auth.m_auth_data;
    g_bp_auth.m_auth_cmd.m_arg = &g_bp_auth;
    g_bp_auth.m_auth_cmd.m_cmd = BP_CMD_READ_SN;

    sm_co_if_reg_recv_callback(_co_if, sm_bp_auth_recv_data, &g_bp_auth);

    return &g_bp_auth;
}
                                
int32_t sm_bp_auth_destroy(sm_bp_auth_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_cb = NULL;
    _impl(_this)->m_cb_arg = NULL;
    _impl(_this)->m_node_id_ctl = NULL;

    _impl(_this)->m_assigning_bp = NULL;
    _impl(_this)->m_assigning_state = BP_ASSIGN_IDLE;

    return 0;
}

/// @brief 
/// @param _this 
/// @param _id 
/// @return 
int32_t sm_bp_auth_start_auth(sm_bp_auth_t* _this, const sm_bp_t* _bp){
    if(!_this){
        return -1;
    }
    
    if(_impl(_this)->m_assigning_bp){
        _impl(_this)->m_node_id_ctl->sm_bp_node_id_deselect(_impl(_this)->m_assigning_bp->m_id);
        sm_bp_reset(_impl(_this)->m_assigning_bp);
    }

    sm_bp_auth_reset(_this);

    _impl(_this)->m_assigning_bp = _bp;
    _impl(_this)->m_auth_cmd.m_id = _bp->m_id;

    elapsed_timer_resetz(&_impl(_this)->m_assigning_timeout, BP_ASSIGNING_TIMEOUT);
    _impl(_this)->m_assigning_state = BP_ASSIGN_WAIT_REQUEST;

    if(_impl(_this)->m_node_id_ctl->sm_bp_node_id_select(_impl(_this)->m_assigning_bp->m_id) < 0){
        if(_impl(_this)->m_cb){
            _impl(_this)->m_cb(_bp->m_id, BP_AUTH_FAILURE, NULL, -1, _impl(_this)->m_cb_arg);
        }
        sm_bp_auth_reset(_this);
        return -1;
    }

    LOG_DBG(TAG,"Starting: Assigning state is switched to BP_ASSIGN_WAIT_REQUEST");

    return 0;
}

int32_t sm_bp_auth_get_bp_authenticating(sm_bp_auth_t* _this){
    return _impl(_this)->m_assigning_bp ? _impl(_this)->m_assigning_bp->m_id : -1;
}

int32_t sm_bp_auth_process(sm_bp_auth_t* _this){
    if(!_this || !_impl(_this)->m_assigning_bp){
        return -1;
    }

    uint8_t nodeId = _impl(_this)->m_assigning_bp->m_id + CAN_NODE_ID_OFFSET;

    switch (_impl(_this)->m_assigning_state) {
        case BP_ASSIGN_WAIT_REQUEST:
        case BP_ASSIGN_SLAVE_SELECT:
        case BP_ASSIGN_WAIT_CONFIRM:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_assigning_timeout)){
                LOG_DBG(TAG, "Bp %d Timeout Assigning progress at step %d",
                            _impl(_this)->m_assigning_bp->m_id,
                            _impl(_this)->m_assigning_state);

                _impl(_this)->m_assigning_state = BP_ASSIGN_FAILURE;
            }
            break;
        case BP_ASSIGN_START:
            if(_impl(_this)->m_node_id_ctl->sm_bp_node_id_deselect(_impl(_this)->m_assigning_bp->m_id) < 0){
                _impl(_this)->m_assigning_state = BP_ASSIGN_FAILURE;
                break;
            }

            sm_co_if_send(_impl(_this)->m_co_if, CAN_NODE_ID_ASSIGN_COBID, NULL, 0, 100);

            _impl(_this)->m_assigning_state = BP_ASSIGN_SLAVE_SELECT;
            elapsed_timer_reset(&_impl(_this)->m_assigning_timeout);
            break;
        case BP_ASSIGN_SLAVE_SELECT_CONFIRM:
            sm_co_if_send(_impl(_this)->m_co_if,
                            CAN_NODE_ID_ASSIGN_COBID,
                            (const uint8_t*)&nodeId,
                            1,
                            100);

            elapsed_timer_reset(&_impl(_this)->m_assigning_timeout);              
            _impl(_this)->m_assigning_state = BP_ASSIGN_WAIT_CONFIRM;

            LOG_DBG(TAG,"Assigning state is switched to WAIT_CONFIRM: %d", _impl(_this)->m_assigning_bp->m_id);
            break;
        case BP_ASSIGN_AUTHORIZING_START:
            if(_impl(_this)->m_node_id_ctl->sm_bp_node_id_select(_impl(_this)->m_assigning_bp->m_id) < 0){
                _impl(_this)->m_assigning_state = BP_ASSIGN_FAILURE;
                break;
            }

            sm_bp_co_set_cmd_force(_impl(_this)->m_bp_co, &_impl(_this)->m_auth_cmd);

            _impl(_this)->m_assigning_state = BP_ASSIGN_AUTHORIZING;
            elapsed_timer_resetz(&_impl(_this)->m_assigning_timeout, BP_ASSIGNING_AUTH_TIMEOUT);

            LOG_DBG(TAG,"Assigning state is switched from BP_ASSIGN_AUTHORIZING_START to BP_ASSIGN_AUTHORIZING: %d", _impl(_this)->m_assigning_bp->m_id);
            break;

        case BP_ASSIGN_AUTHORIZING:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_assigning_timeout)){
                 LOG_ERR(TAG, "Assigning progress FAILURE, reason: Authorizing TIMEOUT: %d", _impl(_this)->m_assigning_bp->m_id);
                _impl(_this)->m_assigning_state = BP_ASSIGN_FAILURE;
                return -1;
            }
            break;

        case BP_ASSIGN_VERIFY_AUTH:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_assigning_timeout)){
                LOG_ERR(TAG, "Assigning progress FAILURE, reason: Authorizing TIMEOUT: %d", _impl(_this)->m_assigning_bp->m_id);
                _impl(_this)->m_assigning_state = BP_ASSIGN_FAILURE;
                return -1;
            }

            if(_impl(_this)->m_assigning_bp->m_data.m_soc < 0){
                break;
            }

            sm_bp_auth_verify_sn(_impl(_this)->m_auth_data, _impl(_this)->m_assigning_bp->m_data.m_sn);
         /*   	LOG_WRN(TAG, "BP Serial Number is INVALID");
                _impl(_this)->m_assigning_state = BP_ASSIGN_FAILURE;
                 return 0;
            }*/

            _impl(_this)->m_assigning_state = BP_ASSIGNED;
            elapsed_timer_reset(&_impl(_this)->m_assigning_bp->m_timeout);

            break;

        case BP_ASSIGNED:
            _impl(_this)->m_assigning_bp->m_is_connected = 1;
            LOG_DBG(TAG, "Assigning SUCCESS, BP info node id: %d, sn: %s, soc: %d, is connected: %d",
                      _impl(_this)->m_assigning_bp->m_id,
                      _impl(_this)->m_assigning_bp->m_data.m_sn,
                    _impl(_this)->m_assigning_bp->m_data.m_soc,
                    _impl(_this)->m_assigning_bp->m_is_connected);
            
            if(_impl(_this)->m_cb){
                _impl(_this)->m_cb(_impl(_this)->m_assigning_bp->m_id,
                                        BP_AUTH_SUCCESS, 
                                        _impl(_this)->m_assigning_bp->m_data.m_sn,
                                        _impl(_this)->m_assigning_bp->m_data.m_soc, 
                                        _impl(_this)->m_cb_arg);    
            } 

            sm_bp_auth_reset(_this);
            break;

        case BP_ASSIGN_FAILURE:
            _impl(_this)->m_node_id_ctl->sm_bp_node_id_deselect(_impl(_this)->m_assigning_bp->m_id);

            elapsed_timer_resetz(&_impl(_this)->m_assigning_timeout, BP_ASSIGNING_WAIT_SLAVE_DESELECT_TIMEOUT);
            _impl(_this)->m_assigning_state = BP_ASSIGN_WAIT_SLAVE_DESELECT;
            break;

        case BP_ASSIGN_WAIT_SLAVE_DESELECT:
            if(!elapsed_timer_get_remain(&_impl(_this)->m_assigning_timeout)){
                if(_impl(_this)->m_cb){
                    _impl(_this)->m_cb(_impl(_this)->m_assigning_bp->m_id, 
                                        BP_AUTH_FAILURE, 
                                        _impl(_this)->m_assigning_bp->m_data.m_sn,
                                        _impl(_this)->m_assigning_bp->m_data.m_soc, 
                                        _impl(_this)->m_cb_arg);    
                }
                sm_bp_auth_reset(_this);
            }
            break;
        case BP_ASSIGN_IDLE:
            break;
    }

    return 0;
}
