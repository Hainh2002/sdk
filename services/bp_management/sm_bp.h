#ifndef SM_BP_H
#define SM_BP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "sm_bp_data.h"
#include "sm_elapsed_timer.h"

#define BP_CMD_QUEUE_SIZE   		32
#define BP_CONNECTION_TIMEOUT		5000

#define BP_DATA_FIELD_NUMBER		6

typedef struct sm_bp{
    int32_t m_id;
    sm_bp_data_t m_data;

    uint8_t m_is_connected;
    elapsed_timer_t m_timeout;

    uint8_t m_data_count;
}sm_bp_t;

typedef struct sm_bp_cmd{
    int32_t m_id;
    SM_BP_CMD m_cmd;
    void* m_data;
    sm_bp_on_cmd_fn_t m_cb;
    void* m_arg;
}sm_bp_cmd_t;

static inline void sm_bp_reset(sm_bp_t* _this){
    sm_bp_reset_data(&_this->m_data);
    _this->m_is_connected = 0;
    elapsed_timer_resetz(&_this->m_timeout, BP_CONNECTION_TIMEOUT);
    _this->m_data_count = 0;
}

static inline void sm_bp_cmd_reset(sm_bp_cmd_t* _this){
    _this->m_id = -1;
    _this->m_cmd = BP_CMD_NUMBER;
    _this->m_data = NULL;
    _this->m_cb = NULL;
    _this->m_arg = NULL;
}

#ifdef __cplusplus
}
#endif


#endif /// SM_BP_H
