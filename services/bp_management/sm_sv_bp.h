#ifndef SM_SV_BP_H
#define SM_SV_BP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sm_bp_data.h"
#include "sm_co_if.h"
//#include "sm_core_co.c"
#include "sm_core_co.h"
#include "sm_bp_auth.h"

#define SM_SV_BP_NUMBER_DEFAULT 3

typedef struct {
    void (*on_bp_connected)(int32_t, const char*, int32_t, void*);
    void (*on_bp_disconnected)(int32_t, const char*, void*);
    void (*on_bp_update_data)(int32_t, const sm_bp_data_t*, void*);
}sm_sv_bp_event_cb_t;

typedef void sm_sv_bp_t;

sm_sv_bp_t* sm_sv_bp_create(int32_t _bp_num, sm_co_t* _co, bool _auth_master, void* _auth_master_if);

int32_t sm_sv_bp_destroy(sm_sv_bp_t* _this);

int32_t sm_sv_bp_reg_event(sm_sv_bp_t* _this, const sm_sv_bp_event_cb_t* _event_cb_fn, void* _arg);

int32_t sm_sv_bp_get_number(sm_sv_bp_t* _this);

const sm_bp_data_t* sm_sv_bp_get_data(sm_sv_bp_t* _this, int32_t _id);

int32_t sm_sv_bp_reset(sm_sv_bp_t* _this, int32_t _id);

int32_t sm_sv_bp_auth(sm_sv_bp_t* _this, 
                        int32_t _id,
                        sm_bp_auth_event_fn_t _cb,
                        void* _arg);

int32_t sm_sv_bp_is_authenticating(sm_sv_bp_t* _this,
                                int32_t _id);

int32_t sm_sv_bp_is_connected(sm_sv_bp_t* _this, int32_t _id);

int32_t sm_sv_bp_get_connected_bp(sm_sv_bp_t* _this, int32_t* _list);

int32_t sm_sv_bp_set_cmd(sm_sv_bp_t* _this,
                        int32_t _id, 
                        SM_BP_CMD _cmd, 
                        void* _data,
                        sm_bp_on_cmd_fn_t _cmd_cb,
                        void* _arg);

int32_t sm_sv_bp_reset_current_cmd(sm_sv_bp_t* _this);                    

int32_t sm_sv_bp_process(sm_sv_bp_t* _this);                        


#ifdef __cplusplus
}
#endif


#endif
