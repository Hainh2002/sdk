//
// Created by vnbk on 28/03/2023.
//
#include <stdlib.h>
#include "sm_boot_master.h"
#include "sm_elapsed_timer.h"

#define TAG "sm_boot_master"

struct sm_boot_master{
    int32_t m_slave_number;
    sm_boot_slave_t* m_slaves[SM_BOOT_SLAVE_MAX_SUPPORT];

    sm_boot_master_event_callback_fn_t m_event_callback;
    void* m_arg;
};

static sm_boot_master_t g_boot_master_default = {
        .m_slave_number = 0,
        .m_slaves = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

static int32_t sm_boot_master_event_finish_download_callback(void* _data, void* _arg){

    sm_boot_slave_t* slave = (sm_boot_slave_t*)_arg;
    //LOG_INFO(TAG, "Upgraded fw SUCCESS: slave id: %d", sm_boot_slave_get_id(slave));

    sm_boot_master_t* boot_master = (sm_boot_master_t*)sm_boot_slave_get_master(slave);
    if(boot_master && boot_master->m_event_callback){
        boot_master->m_event_callback(0, sm_boot_slave_get_id(slave), boot_master->m_arg);
    }

    sm_boot_master_remove_slave(boot_master, sm_boot_slave_get_id(slave));
    return 0;
}
static int32_t sm_boot_master_event_abort_callback(void* _data, void* _arg){
    sm_boot_slave_t* slave = (sm_boot_slave_t*)_arg;
    //LOG_ERROR(TAG, "Upgraded fw FAILURE on slave id: %d, Abort progress", sm_boot_slave_get_id(slave));
    sm_boot_master_t* boot_master = (sm_boot_master_t*)sm_boot_slave_get_master(slave);
    if(!boot_master){
        return -1;
    }
    if(boot_master->m_event_callback){
        boot_master->m_event_callback(-1, sm_boot_slave_get_id(slave), boot_master->m_arg);
    }

    sm_boot_master_remove_slave(boot_master, sm_boot_slave_get_id(slave));
    return 0;
}

sm_boot_master_t* sm_boot_master_create(sm_boot_master_event_callback_fn_t _fn, void* _arg){
    sm_boot_master_t* obj = (sm_boot_master_t*)malloc(sizeof (sm_boot_master_t));
    if(!obj){
        //LOG_ERROR(TAG, "Could NOT create boot-core master, Allocated memory FAILURE");
        return NULL;
    }
    for (int index = 0; index < SM_BOOT_SLAVE_MAX_SUPPORT; index++){
        obj->m_slaves[index] = NULL;
    }
    obj->m_slave_number = 0;
    obj->m_event_callback = _fn;
    obj->m_arg = _arg;

    return obj;
}

sm_boot_master_t* sm_boot_master_create_default(sm_boot_master_event_callback_fn_t _fn, void* _arg){
    for (int index = 0; index < SM_BOOT_SLAVE_MAX_SUPPORT; index++){
        g_boot_master_default.m_slaves[index] = NULL;
    }
    g_boot_master_default.m_slave_number = 0;
    g_boot_master_default.m_event_callback = _fn;
    g_boot_master_default.m_arg = _arg;

    return &g_boot_master_default;
}

int32_t sm_boot_master_destroy(sm_boot_master_t* _self){
    if(!_self){
        return -1;
    }
    for(int index = 0; index < _self->m_slave_number; index++){
        free(_self->m_slaves[index]);
    }
    free(_self);
}

int32_t sm_boot_master_add_slave(sm_boot_master_t* _self,
                                 int32_t _slave_id,
                                 sm_boot_input_if_t* _fw_input,
                                 sm_boot_output_if_t* _fw_output){
    sm_boot_slave_t* slave = NULL;
    int32_t index;
    if(!_self || !_fw_input || !_fw_output){
        return -1;
    }

    for(index = 0; index < SM_BOOT_SLAVE_MAX_SUPPORT; index++){
        slave = _self->m_slaves[index];
        if(slave && sm_boot_slave_get_id(slave) == _slave_id){
            sm_boot_slave_update_if(slave, _fw_input, _fw_output);
            //LOG_INFO(TAG, "Boot Master update boot-core slave info");
            return 0;
        }
    }

    slave = sm_boot_slave_create(_slave_id, _fw_input, _fw_output, _self);
    if(!slave){
        //LOG_ERROR(TAG, "Could NOT add boot-core slave, Created boot-core slave FAILURE");
        return -1;
    }

    for(index = 0; index < SM_BOOT_SLAVE_MAX_SUPPORT; index++){
        if(!_self->m_slaves[index]){
            _self->m_slaves[index] = slave;

            sm_boot_slave_reg_event(slave, SM_BOOT_SLAVE_CONFIRM_FINISH_DOWNLOAD, sm_boot_master_event_finish_download_callback, slave);
            sm_boot_slave_reg_event(slave, SM_BOOT_SLAVE_CONFIRM_ABORT, sm_boot_master_event_abort_callback, slave);

            _self->m_slave_number++;
            break;
        }
    }

    return 0;
}

int32_t sm_boot_master_remove_slave(sm_boot_master_t* _self, int32_t _slave_id){
    int32_t index = 0;
    sm_boot_slave_t* slave = NULL;
    if(!_self){
        return -1;
    }
    for(index = 0; index < SM_BOOT_SLAVE_MAX_SUPPORT; index++){
        slave = _self->m_slaves[index];
        if(sm_boot_slave_get_id(slave) == _slave_id){
            sm_boot_slave_destroy(slave);
            _self->m_slaves[index] = NULL;
            _self->m_slave_number--;
            //LOG_INFO(TAG, "Boot Master remove boot-core slave");
            return 0;
        }
    }
    return -1;
}

int32_t sm_boot_master_process(sm_boot_master_t* _self){
    int32_t index = 0;
    sm_boot_slave_t* slave = NULL;
    if(!_self || !_self->m_slave_number){
        return -1;
    }
    for(index = 0; index < SM_BOOT_SLAVE_MAX_SUPPORT; index++) {
        slave = _self->m_slaves[index];
        if(slave){
            sm_boot_slave_process(slave);
        }
    }
    return 0;
}
