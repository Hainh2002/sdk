//
// Created by vnbk on 28/03/2023.
//
#include <stdlib.h>
#include <stdio.h>

#include "sm_boot_slave.h"
#include "sm_elapsed_timer.h"
#include "sm_logger.h"

#define TAG "sm_boot_slave"

#define SM_BOOT_SLAVE_REQUEST_UPGRADE_PERIOD            6500
#define SM_BOOT_SLAVE_IDLE_TIMEOUT                      10000 //ms
#define SM_BOOT_SLAVE_WAITING_CONFIRM_FW_INFO_TIMEOUT   (5*1000)
#define SM_BOOT_SLAVE_WAITING_CONFIRM_SEG_TIMEOUT       (20*1000)
#define SM_BOOT_SLAVE_WAITING_FULL_SEGMENT_TIMEOUT      (10*1000)

#define SM_BOOT_SLAVE_REPEAT_MAX    5

typedef enum{
    SM_BOOT_SLAVE_STATE_IDLE,
    SM_BOOT_SLAVE_STATE_REQUEST_UPGRADE,
    SM_BOOT_SLAVE_STATE_SEND_FW_INFO,
    SM_BOOT_SLAVE_STATE_SEND_SEG,
    SM_BOOT_SLAVE_STATE_FINISH_DOWNLOAD,
    SM_BOOT_SLAVE_STATE_ABORT,
    SM_BOOT_SLAVE_STATE_NUMBER,
}SM_BOOT_SLAVE_STATE;

typedef struct sm_boot_slave_event{
    sm_boot_slave_event_callback_fn_t m_callback;
    void* m_arg;
}sm_boot_slave_event_t;

struct sm_boot_slave{
    int32_t m_id;
    sm_boot_input_if_t* m_boot_if_input;
    sm_boot_output_if_t* m_boot_if_output;

    sm_segment_t* m_segment;
    uint8_t m_waiting_seg;
    sm_fw_signature_t m_new_fw_signature;

    SM_BOOT_SLAVE_STATE m_state;
    sm_boot_slave_event_t m_events[SM_BOOT_SLAVE_EVENT_NUMBER];

    elapsed_timer_t m_timeout;
    uint8_t m_repeat;

    void* m_master;
};

static sm_boot_slave_t g_boot_slave_default = {
        .m_id = -1,
        .m_boot_if_input = NULL,
        .m_boot_if_output = NULL,
        .m_state = SM_BOOT_SLAVE_STATE_IDLE,
        .m_segment = NULL,
        .m_repeat = 0,
        .m_waiting_seg = 0,
        .m_master = NULL
};

static int32_t sm_boot_slave_event_slave_confirm_ready(void* _data, void* _arg);
static int32_t sm_boot_slave_event_slave_confirm_fw_info(void* _data, void* _arg);
static int32_t sm_boot_slave_event_slave_confirm_seg(void* _data, void* _arg);
static int32_t sm_boot_slave_event_slave_confirm_finish_download(void* _data, void* _arg);
static int32_t sm_boot_slave_event_slave_confirm_abort(void* _data, void* _arg);

sm_boot_slave_t* sm_boot_slave_create(int32_t _id, sm_boot_input_if_t* _input, sm_boot_output_if_t* _output, void* _master){
    sm_boot_slave_t* obj = NULL;
    sm_segment_t* seg = NULL;
    if(_id < 0 || !_input || !_output){
        return NULL;
    }
    obj = (sm_boot_slave_t*)malloc(sizeof(sm_boot_slave_t));
    if(!obj){
        return NULL;
    }

    seg = sm_seg_create();
    if(!seg){
        free(obj);
        return NULL;
    }

    obj->m_segment = seg;

    obj->m_id = _id;
    obj->m_state = SM_BOOT_SLAVE_STATE_IDLE;
    elapsed_timer_resetz(&obj->m_timeout, 0);
    obj->m_repeat = 0;
    obj->m_waiting_seg = 0;
    obj->m_boot_if_output = _output;
    obj->m_boot_if_input = _input;
    obj->m_master = _master;

    _input->init();

    _output->m_proc->init(_output);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_READY, sm_boot_slave_event_slave_confirm_ready, obj);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_FW_INFO, sm_boot_slave_event_slave_confirm_fw_info, obj);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_SEG, sm_boot_slave_event_slave_confirm_seg, obj);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_FINISH_DOWNLOAD, sm_boot_slave_event_slave_confirm_finish_download, obj);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_FINISH_DOWNLOAD, sm_boot_slave_event_slave_confirm_abort, obj);

    printf("done input init\n");

    return obj;
}

sm_boot_slave_t* sm_boot_slave_create_default(int32_t id, sm_boot_input_if_t* _input, sm_boot_output_if_t* _output, void* _master){
    sm_segment_t* seg = NULL;
    if(!_input || !_output){
        return NULL;
    }
    seg = sm_seg_create();
    if(!seg){
        return NULL;
    }

    g_boot_slave_default.m_segment = seg;
    g_boot_slave_default.m_id = id;

    g_boot_slave_default.m_boot_if_input = _input;
    g_boot_slave_default.m_boot_if_output = _output;
    g_boot_slave_default.m_master = _master;

    _input->init();
    _output->m_proc->init(_output);

    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_READY, sm_boot_slave_event_slave_confirm_ready, &g_boot_slave_default);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_FW_INFO, sm_boot_slave_event_slave_confirm_fw_info, &g_boot_slave_default);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_SEG, sm_boot_slave_event_slave_confirm_seg, &g_boot_slave_default);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_FINISH_DOWNLOAD, sm_boot_slave_event_slave_confirm_finish_download, &g_boot_slave_default);
    _output->m_proc->reg_event_callback(_output, SM_BOOT_SLAVE_CONFIRM_ABORT, sm_boot_slave_event_slave_confirm_abort, &g_boot_slave_default);

    return &g_boot_slave_default;
}

int32_t sm_boot_slave_destroy(sm_boot_slave_t* _self){
    if(!_self){
        return -1;
    }
    if(_self->m_boot_if_input){
        _self->m_boot_if_input->free();
        _self->m_boot_if_input = NULL;
    }

    if(_self->m_boot_if_output){
        _self->m_boot_if_output->m_proc->free(_self->m_boot_if_output);
        _self->m_boot_if_output = NULL;
    }

    if(_self->m_segment){
        sm_seg_destroy(_self->m_segment);
        _self->m_segment = NULL;
    }

    free(_self);
    return 0;
}

void* sm_boot_slave_get_master(sm_boot_slave_t* _self){
    if(!_self){
        return NULL;
    }
    return _self->m_master;
}

int32_t sm_boot_slave_get_id(sm_boot_slave_t* _self){
    if(!_self){
        return -1;
    }
    return _self->m_id;
}

int32_t sm_boot_slave_update_if(sm_boot_slave_t* _self,  sm_boot_input_if_t* _input, sm_boot_output_if_t* _output){
    if(!_self || !_input || !_output){
        return -1;
    }
    _self->m_boot_if_input = _input;
    _self->m_boot_if_output = _output;

    return 0;
}

int32_t sm_boot_slave_reg_event(sm_boot_slave_t* _self, SM_BOOT_SLAVE_EVENT _event, sm_boot_slave_event_callback_fn_t _fn_callback, void* _arg){
    if(!_self || !_fn_callback || _event >= SM_BOOT_SLAVE_EVENT_NUMBER){
        return -1;
    }
    _self->m_events[_event].m_callback = _fn_callback;
    _self->m_events[_event].m_arg = _arg;

    return 0;
}

int32_t sm_boot_slave_process(sm_boot_slave_t* _self){
    sm_fw_signature_t signature;
    int32_t ret = -1;
    if(!_self){
        return -1;
    }

    if(_self->m_boot_if_input->process){
        _self->m_boot_if_input->process();
    }

    if(_self->m_boot_if_output->m_proc->process){
        _self->m_boot_if_output->m_proc->process(_self->m_boot_if_output);
    }

    switch (_self->m_state) {
        case SM_BOOT_SLAVE_STATE_IDLE:
            if(!elapsed_timer_get_remain(&_self->m_timeout)){
                elapsed_timer_resetz(&_self->m_timeout, SM_BOOT_SLAVE_IDLE_TIMEOUT);
                if(_self->m_boot_if_input->get_fw_info(&signature) < 0){
                    printf( "Boot Input FAILURE, Could NOT get fw info");
                    if(_self->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_callback){
                        _self->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_callback(NULL, _self->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_arg);
                    }

                    _self->m_repeat++;
                    if(_self->m_repeat >= SM_BOOT_SLAVE_REPEAT_MAX){
                        _self->m_state = SM_BOOT_SLAVE_STATE_ABORT;
                    }

                    return -1;
                }else{
                    sm_fw_signature_clone(&_self->m_new_fw_signature, &signature);

                    _self->m_state = SM_BOOT_SLAVE_STATE_REQUEST_UPGRADE;
                    elapsed_timer_resetz(&_self->m_timeout, 0);
                    _self->m_repeat = 0;
                }
            }
            break;
        case SM_BOOT_SLAVE_STATE_REQUEST_UPGRADE:
            if(!elapsed_timer_get_remain(&_self->m_timeout)){
                _self->m_boot_if_output->m_proc->request_upgrade(_self->m_boot_if_output, _self->m_id);

                _self->m_repeat++;
                if(_self->m_repeat >= SM_BOOT_SLAVE_REPEAT_MAX){
                    _self->m_state = SM_BOOT_SLAVE_STATE_ABORT;
                }

                elapsed_timer_resetz(&_self->m_timeout, SM_BOOT_SLAVE_REQUEST_UPGRADE_PERIOD);
            }
            break;
        case SM_BOOT_SLAVE_STATE_SEND_FW_INFO:
            if(!elapsed_timer_get_remain(&_self->m_timeout)){
                _self->m_boot_if_output->m_proc->set_fw_info(_self->m_boot_if_output, &_self->m_new_fw_signature);

                elapsed_timer_resetz(&_self->m_timeout, SM_BOOT_SLAVE_WAITING_CONFIRM_FW_INFO_TIMEOUT);
                _self->m_repeat++;
                if(_self->m_repeat >= SM_BOOT_SLAVE_REPEAT_MAX){
                    LOG_ERR("CO boot", "Slave abort fw info\n");
                    _self->m_state = SM_BOOT_SLAVE_STATE_ABORT;
                }
            }
            break;
        case SM_BOOT_SLAVE_STATE_SEND_SEG:
            if(!_self->m_waiting_seg){
                ret = _self->m_boot_if_input->get_seg_fw(_self->m_segment);
                if(ret < 0){
                    printf( "Read segment fw from fw source FAILURE\n");
                    _self->m_state = SM_BOOT_SLAVE_STATE_ABORT;
                }else if(ret == 0){
//                    printf( "Waiting full segment from HOST");
                }else{
                    _self->m_waiting_seg = 1;
                    elapsed_timer_resetz(&_self->m_timeout, SM_BOOT_SLAVE_WAITING_CONFIRM_SEG_TIMEOUT);

                    printf( "Set segment info to Boot Output\n");

                    if(_self->m_boot_if_output->m_proc->set_seg_fw(_self->m_boot_if_output, _self->m_segment) < 0){
                        printf( "Write segment fw to fw destination FAILURE");
                        return -1;
                    }

                    _self->m_repeat++;
                    if(_self->m_repeat >= SM_BOOT_SLAVE_REPEAT_MAX){
                        LOG_ERR("CO boot", "Slave abort segment\n");
                        _self->m_state = SM_BOOT_SLAVE_STATE_ABORT;
                    }
                }
            }
            if(!elapsed_timer_get_remain(&_self->m_timeout)){
                LOG_ERR("CO boot", "Slave abort m_timeout send segment\n");
                _self->m_state = SM_BOOT_SLAVE_STATE_ABORT;
                elapsed_timer_reset(&_self->m_timeout);
            }
            break;
        case SM_BOOT_SLAVE_STATE_FINISH_DOWNLOAD:
            if(_self->m_events[SM_BOOT_SLAVE_CONFIRM_FINISH_DOWNLOAD].m_callback){
                _self->m_events[SM_BOOT_SLAVE_CONFIRM_FINISH_DOWNLOAD].m_callback(NULL, _self);
            }
            break;
        case SM_BOOT_SLAVE_STATE_ABORT:
            if(_self->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_callback){
                _self->m_events[SM_BOOT_SLAVE_CONFIRM_ABORT].m_callback(NULL, _self);
            }
            break;
        default:
            _self->m_state = SM_BOOT_SLAVE_STATE_ABORT;
            break;
    }

    return 0;
}

static int32_t sm_boot_slave_event_slave_confirm_ready(void* _data, void* _arg){
    sm_boot_slave_t* slave = (sm_boot_slave_t*)_arg;
    if(!slave){
        printf( "Slave is NOT exits, Please check again");
        return -1;
    }
    uint8_t ready = *(uint8_t*)_data;
    if(ready){
        slave->m_state = SM_BOOT_SLAVE_STATE_SEND_SEG;
        elapsed_timer_resetz(&slave->m_timeout, SM_BOOT_SLAVE_WAITING_FULL_SEGMENT_TIMEOUT);
        printf("Slave confirm ready to Upgrading: %d\n", slave->m_id);
    }
    return 0;
}

static int32_t sm_boot_slave_event_slave_confirm_fw_info(void* _data, void* _arg){
    sm_boot_slave_t* slave = (sm_boot_slave_t*)_arg;
    uint8_t success = *(uint8_t*)_data;
    if(!slave){
        printf("Slave is NOT exits, Please check again");
        return -1;
    }

    if(success){
        slave->m_state = SM_BOOT_SLAVE_STATE_FINISH_DOWNLOAD;
        printf("Slave confirm fw info SUCCESS: %d\n", slave->m_id);
    }else{
        printf("Slave confirm fw info FAILURE: %d\n", slave->m_id);
        elapsed_timer_pass(&slave->m_timeout);
    }
    return 0;
}

static int32_t sm_boot_slave_event_slave_confirm_seg(void* _data, void* _arg){
    sm_boot_slave_t* slave = (sm_boot_slave_t*)_arg;
    uint8_t success = *(uint8_t*)_data;
    if(!slave){
        printf( "Slave is NOT exits, Please check again");
        return -1;
    }
    if(success){
        printf("Slave confirm received segment: %d SUCCESS: slave id: %d\n", slave->m_segment->m_index, slave->m_id);

        if(slave->m_boot_if_input->seg_upgrade_completed){
            slave->m_boot_if_input->seg_upgrade_completed(slave->m_segment->m_index);
        }

        slave->m_segment->m_index++;
        if(slave->m_boot_if_input->get_total_seg() <= slave->m_segment->m_index){
            printf("Finish download fw, Download fw SUCCESS: %d\n", slave->m_id);
            slave->m_state = SM_BOOT_SLAVE_STATE_SEND_FW_INFO;
            elapsed_timer_pass(&slave->m_timeout);

            slave->m_waiting_seg = 0;
            slave->m_repeat = 0;
            return 0;
        }
        slave->m_repeat = 0;
        elapsed_timer_reset(&slave->m_timeout);
    }else{
        LOG_ERR("CO boot", "Slave %d confirm received segment: FAILURE\n", slave->m_id);
    }

    slave->m_waiting_seg = 0;
    return 0;
}

static int32_t sm_boot_slave_event_slave_confirm_finish_download(void* _data, void* _arg){
    (void)_data;
    sm_boot_slave_t* slave = (sm_boot_slave_t*)_arg;
    if(!slave){
        printf( "Slave is NOT exits, Please check again");
        return -1;
    }

    slave->m_state = SM_BOOT_SLAVE_STATE_FINISH_DOWNLOAD;
    printf("Finish download fw, Download fw SUCCESS\n");
    return 0;
}

static int32_t sm_boot_slave_event_slave_confirm_abort(void* _data, void* _arg){
    (void)_data;
    sm_boot_slave_t* slave = (sm_boot_slave_t*)_arg;
    if(!slave){
        printf( "Slave is NOT exits, Please check again");
        return -1;
    }
    slave->m_state = SM_BOOT_SLAVE_STATE_ABORT;
    printf("Finish download fw, Download fw ABORT\n");
    return 0;
}