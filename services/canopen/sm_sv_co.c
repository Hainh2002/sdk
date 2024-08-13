//
// Created by vuonglk on 07/06/2024.
//

#include "sm_sv_co.h"
#include "app_co_init.h"
#include "sm_vector.h"
#include "sm_elapsed_timer.h"

#define SM_SV_CO_MAX_CMD_SUPPORT 64

typedef struct sm_sv_co_cmd{
    SM_SV_CO_CMD m_cmd;
    uint32_t m_timeout;
    sm_sv_co_cmd_callback m_callback;
    void* m_arg;
}sm_sv_co_cmd_t;

typedef struct sm_sv_co_impl{
    CO* m_co_device;
    sm_vector_t* m_cmd_queue;
    sm_sv_co_cmd_t* m_current_cmd;
    elapsed_timer_t m_cmd_timeout;
    bool m_is_waiting;
}sm_sv_co_impl_t;

#define impl(x) ((sm_sv_co_impl_t*)(x))

int32_t sm_sv_co_reset_cmd(sm_sv_co_t* _this){
    sm_sv_co_impl_t* this = this = impl(_this);
    if (this == NULL) return -1;
    if(this->m_current_cmd == NULL) return 0;

    this->m_current_cmd = NULL;
    this->m_is_waiting = false;
    CO_SDO_reset_status(&this->m_co_device->sdo_client);

    return 0;
}

sm_sv_co_t* sm_sv_co_create(){
    sm_sv_co_impl_t* this = malloc(sizeof(sm_sv_co_impl_t));
    if (this == NULL) return NULL;

    this->m_cmd_queue = sm_vector_create(SM_SV_CO_MAX_CMD_SUPPORT, sizeof(sm_sv_co_cmd_t));
    if (this->m_cmd_queue == NULL) return NULL;

    this->m_co_device = &CO_DEVICE;

    return this;
}

int32_t sm_sv_co_push_cmd(sm_sv_co_t* _this, uint32_t _id, SM_SV_CO_CMD _cmd, uint32_t _timeout,
                          sm_sv_co_cmd_callback _callback, void *_arg, bool _force){

    sm_sv_co_impl_t* this = impl(_this);
    if(!this) return -1;

    sm_sv_co_cmd_t cmd = {.m_cmd = _cmd, .m_timeout = _timeout, .m_callback = _callback, .m_arg = _arg};

    if (sm_vector_is_full(this->m_cmd_queue)) {
        return -1;
    }

    if(_force){
        if(this->m_current_cmd){
            if (sm_vector_push_font(this->m_cmd_queue, &this->m_current_cmd) < 0) {
                return -1;
            }
        }

        if (sm_vector_push_font(this->m_cmd_queue, &cmd) < 0) {
            return -1;
        }

        sm_sv_co_reset_cmd(this);

        printf("Force CMD %d to queue\n", _cmd);

    }else{
        if (sm_vector_push_back(this->m_cmd_queue, &cmd) < 0) {
            return -1;
        }

        printf("Push CMD %d to queue\n", _cmd);
    }

    return 0;
}

int32_t sm_sv_co_process(sm_sv_co_t* _this){
    sm_sv_co_impl_t* this = impl(_this);
    if(!this) return -1;

    app_co_process();

    if(this->m_current_cmd){
        sm_sv_co_cmd_t* cmd = this->m_current_cmd;
        if(this->m_is_waiting){
            if(!elapsed_timer_get_remain(&this->m_cmd_timeout)){
                if(cmd->m_callback){
                    cmd->m_callback(cmd->m_cmd, SM_SV_CO_ERROR_TIMEOUT, cmd->m_arg);
                }
                sm_sv_co_reset_cmd(this);
                return 0;
            }
            CO_SDO_return_t status = CO_SDO_get_status(&this->m_co_device->sdo_client);
            if(status == CO_SDO_RT_abort){
                printf("CMD %d error abort\n", cmd->m_cmd);
                if(cmd->m_callback){
                    cmd->m_callback(cmd->m_cmd, SM_SV_CO_ERROR_ABORT, cmd->m_arg);
                }
                sm_sv_co_reset_cmd(this);
                return 0;
            }else if(status == CO_SDO_RT_success){
                printf("CMD %d success\n", cmd->m_cmd);
                if(cmd->m_callback){
                    cmd->m_callback(cmd->m_cmd, SM_SV_CO_ERROR_NONE, cmd->m_arg);
                }
                sm_sv_co_reset_cmd(this);
                return 0;
            }
        }else{
            this->m_is_waiting = true;
        }
    }



    return 0;
}


