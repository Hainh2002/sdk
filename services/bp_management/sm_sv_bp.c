#include "sm_sv_bp.h"
#include "sm_bp.h"
#include "sm_bp_auth.h"
#include "sm_bp_co.h"
#include "sm_elapsed_timer.h"
#include "sm_logger.h"


#define _impl(p)    ((sm_sv_bp_impl_t*)(p))

#define TAG "SM_SV_BP"

#define SM_SV_BP_EVENT_CB_MAX       10


typedef struct {
    int32_t m_numb;
    sm_bp_t m_bps[SM_SV_BP_NUMBER_DEFAULT];

    sm_co_t* m_co;

    sm_bp_co_t* m_bp_co;

    sm_bp_auth_t* m_auth;

    sm_bp_cmd_t m_cmds[BP_CMD_QUEUE_SIZE];
    uint8_t m_cmd_head;
    uint8_t m_cmd_tail;
    sm_bp_cmd_t* m_current_cmd;

    sm_bp_auth_event_fn_t m_force_auth_event; 
    void* m_force_auth_arg;
    uint8_t m_force_auth_id;

    struct {
        const sm_sv_bp_event_cb_t* m_event_cb;
        void* m_event_arg;
    }m_event_handlers[SM_SV_BP_EVENT_CB_MAX];

}sm_sv_bp_impl_t;


static sm_sv_bp_impl_t g_sv_bp_default = {
    .m_numb = SM_SV_BP_NUMBER_DEFAULT,
    .m_event_handlers = {NULL, NULL},
    .m_auth = NULL,
    .m_cmd_head = 0,
    .m_cmd_tail = 0,
    .m_current_cmd = NULL,
    .m_co = NULL
};

static void sm_bp_reset_force_assigning(sm_sv_bp_impl_t* _this){
    _this->m_force_auth_arg = NULL;
    _this->m_force_auth_id = -1;
    _this->m_force_auth_event = NULL;
}

 static void sm_bp_auth_event_handle(int32_t _id, SM_BP_AUTH_EVENT _event, const char* _sn, int32_t _soc, void* _arg){
    sm_sv_bp_impl_t* bp_service = (sm_sv_bp_impl_t*)(_arg);
    if(!bp_service){
        return;
    }
    sm_bp_t* bp = &bp_service->m_bps[_id];
    if(_event == BP_AUTH_SUCCESS){
        bp->m_is_connected = 1;
        elapsed_timer_reset(&bp->m_timeout);

        for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
            if(bp_service->m_event_handlers[index].m_event_cb && bp_service->m_event_handlers[index].m_event_cb->on_bp_connected){
                bp_service->m_event_handlers[index].m_event_cb->on_bp_connected(_id, _sn, _soc, bp_service->m_event_handlers[index].m_event_arg);
            }
        }
    }

    if(_id == bp_service->m_force_auth_id && bp_service->m_force_auth_event){
        bp_service->m_force_auth_event(_id,
                                        _event,
                                        _sn,
                                        _soc, 
                                        bp_service->m_force_auth_arg);

        sm_bp_reset_force_assigning(bp_service);
    }
 }
 
 static void sm_bp_co_update_data(int32_t _id, const sm_bp_data_t* _data, void* _arg){
    sm_sv_bp_impl_t* bp_service = (sm_sv_bp_impl_t*)(_arg);
    if(!bp_service){
        return;
    }
    for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
        if(bp_service->m_event_handlers[index].m_event_cb && bp_service->m_event_handlers[index].m_event_cb->on_bp_update_data){
            bp_service->m_event_handlers[index].m_event_cb->on_bp_update_data(_id, _data, bp_service->m_event_handlers[index].m_event_arg);
        }
    }
 }


sm_sv_bp_t* sm_sv_bp_create(int32_t _bp_num, sm_co_t* _co, bool _auth_master, void* _auth_master_if){
    if(_bp_num < 0 || !_co){
        LOG_ERR(TAG, "Created BP Service FAILURE, Parameter INVALID");
        return NULL;
    }
    g_sv_bp_default.m_numb = _bp_num;

    for(uint8_t index = 0; index < SM_SV_BP_NUMBER_DEFAULT; index++){
        g_sv_bp_default.m_bps[index].m_id = index;
    	sm_bp_reset(&g_sv_bp_default.m_bps[index]);
    }

    g_sv_bp_default.m_cmd_head = 0;
    g_sv_bp_default.m_cmd_tail = 0;
    g_sv_bp_default.m_current_cmd = NULL;
    for(uint8_t index = 0; index < BP_CMD_QUEUE_SIZE; index++){
        g_sv_bp_default.m_cmds[index].m_arg = NULL;
        g_sv_bp_default.m_cmds[index].m_cb = NULL;
        g_sv_bp_default.m_cmds[index].m_cmd = BP_CMD_NUMBER;
        g_sv_bp_default.m_cmds[index].m_id = -1;
        g_sv_bp_default.m_cmds[index].m_data = NULL;
    }

    g_sv_bp_default.m_co = _co;

    g_sv_bp_default.m_bp_co = sm_bp_co_create(_co,
                                            g_sv_bp_default.m_bps,
                                            g_sv_bp_default.m_numb,
                                            sm_bp_co_update_data,
                                            &g_sv_bp_default);

    if(!g_sv_bp_default.m_bp_co){
        LOG_ERR(TAG, "Created BP Service FAILURE, Could NOT create CANOPEN");
        return NULL;
    }

    if(_auth_master){
    	g_sv_bp_default.m_auth = sm_bp_auth_create(sm_co_get_co_if(_co),
													g_sv_bp_default.m_bp_co,
                                                   _auth_master_if,
                                                   sm_bp_auth_event_handle,
                                                   &g_sv_bp_default);
    }

    return &g_sv_bp_default;
}

int32_t sm_sv_bp_destroy(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    return 0;
}

int32_t sm_sv_bp_reg_event(sm_sv_bp_t* _this, const sm_sv_bp_event_cb_t* _event_cb_fn, void* _arg){
    if(!_this){
        return -1;
    }
    for(int index = 0; index < SM_SV_BP_EVENT_CB_MAX; index++){
        if(_impl(_this)->m_event_handlers[index].m_event_cb == NULL){
            _impl(_this)->m_event_handlers[index].m_event_cb = _event_cb_fn;
            _impl(_this)->m_event_handlers[index].m_event_arg = _arg;
            return 0;
        }
    }
    return -1;
}

int32_t sm_sv_bp_get_number(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    return _impl(_this)->m_numb;
}

const sm_bp_data_t* sm_sv_bp_get_data(sm_sv_bp_t* _this, int32_t _id){
    if(!_this || _impl(_this)->m_numb <= _id){
        return NULL;
    }
    return &_impl(_this)->m_bps[_id].m_data;
}

int32_t sm_sv_bp_reset(sm_sv_bp_t* _this, int32_t _id){
    if(!_this){
        return -1;
    }
    sm_bp_reset(&_impl(_this)->m_bps[_id]);
    return 0;
}

int32_t sm_sv_bp_auth(sm_sv_bp_t* _this, 
                      int32_t _id,
                      sm_bp_auth_event_fn_t _cb,
                      void* _arg){
    if(!_this){
        return -1;
    }            
    _impl(_this)->m_force_auth_id = _id;
    _impl(_this)->m_force_auth_event = _cb;
    _impl(_this)->m_force_auth_arg = _arg;

    return sm_bp_auth_start_auth(_impl(_this)->m_auth, &_impl(_this)->m_bps[_id]);
}

int32_t sm_sv_bp_is_authenticating(sm_sv_bp_t* _this,
                                int32_t _id){
    if(!_this){
        return -1;
    }
    if(_impl(_this)->m_bps[_id].m_is_connected){
        return 0;
    }
    return _id == sm_bp_auth_get_bp_authenticating(_impl(_this)->m_auth);
}

int32_t sm_sv_bp_is_connected(sm_sv_bp_t* _this, int32_t _id){
    if(!_this || _id >= _impl(_this)->m_numb){
        return -1;
    }
    return _impl(_this)->m_bps[_id].m_is_connected;
}

int32_t sm_sv_bp_get_connected_bp(sm_sv_bp_t* _this, int32_t* _list){
    if(!_this){
        return -1;
    }
    int32_t count = 0;
    for(int32_t index = 0; index < _impl(_this)->m_numb; index++){
        if(_impl(_this)->m_bps[index].m_is_connected){
            _list[count++] = index;
        }
    }
    return count;
}

int32_t sm_sv_bp_set_cmd(sm_sv_bp_t* _this,
                        int32_t _id, 
                        SM_BP_CMD _cmd, 
                        void* _data,
                        sm_bp_on_cmd_fn_t _cmd_cb,
                        void* _arg){
    if(!_this || !_impl(_this)->m_bps[_id].m_is_connected){
        return -1;
    }
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_id = _id;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_cmd = _cmd;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_data = _data;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_cb = _cmd_cb;
    _impl(_this)->m_cmds[_impl(_this)->m_cmd_head].m_arg = _arg;

    _impl(_this)->m_cmd_head++;
    if(_impl(_this)->m_cmd_head >= BP_CMD_QUEUE_SIZE){
        _impl(_this)->m_cmd_head = 0;
    }
    return 0;
}

int32_t sm_sv_bp_reset_current_cmd(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }
    _impl(_this)->m_current_cmd = NULL;
    return 0;
}                   

int32_t sm_sv_bp_process(sm_sv_bp_t* _this){
    if(!_this){
        return -1;
    }

    if(_impl(_this)->m_auth){
        sm_bp_auth_process(_impl(_this)->m_auth);
    }

    sm_bp_t* bp = NULL;
    for(int32_t index = 0; index < _impl(_this)->m_numb; index++){
        bp = &_impl(_this)->m_bps[index];
        if(!bp->m_is_connected){
            continue;
        }

        if(!elapsed_timer_get_remain(&bp->m_timeout)){

            for(int i = 0; i < SM_SV_BP_EVENT_CB_MAX; i++){
                if(_impl(_this)->m_event_handlers[index].m_event_cb && _impl(_this)->m_event_handlers[i].m_event_cb->on_bp_disconnected){
                    _impl(_this)->m_event_handlers[index].m_event_cb->on_bp_disconnected(bp->m_id, bp->m_data.m_sn,
                                                                                         _impl(_this)->m_event_handlers[index].m_event_arg);
                }
            }

            sm_bp_reset(bp);

            for(int i = 0; i < SM_SV_BP_EVENT_CB_MAX; i++){
                if(_impl(_this)->m_event_handlers[index].m_event_cb && _impl(_this)->m_event_handlers[i].m_event_cb->on_bp_update_data){
                    _impl(_this)->m_event_handlers[index].m_event_cb->on_bp_update_data(bp->m_id, &bp->m_data,
                                                                                         _impl(_this)->m_event_handlers[index].m_event_arg);
                }
            }
        }
    }

    if(!sm_bp_co_is_busy(_impl(_this)->m_co) && !_impl(_this)->m_current_cmd && _impl(_this)->m_cmd_head != _impl(_this)->m_cmd_tail){
        _impl(_this)->m_current_cmd = &_impl(_this)->m_cmds[_impl(_this)->m_cmd_tail];

        _impl(_this)->m_cmd_tail++;
        if(_impl(_this)->m_cmd_tail >= BP_CMD_QUEUE_SIZE){
            _impl(_this)->m_cmd_tail = 0;
        }
        LOG_DBG(TAG, "CURRENT_CMD: ID: %d, CMD: %d", _impl(_this)->m_current_cmd->m_id, _impl(_this)->m_current_cmd->m_cmd);
        /// Process CMD
        sm_bp_co_set_cmd(_impl(_this)->m_co, _impl(_this)->m_current_cmd);
    }

    return 0;
}                   
