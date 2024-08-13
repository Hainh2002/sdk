//
// Created by vnbk on 24/04/2024.
//
#include <unistd.h>
#include <time.h>

#include "sm_core_sch.h"

#include "sm_sv_bp.h"
#include "sm_bp_data.h"
#include "sm_bp_auth.h"

#include "sm_logger.h"

#define MAIN_TAG    "MAIN_TAG"

#define BP_ID_DEFAULT   0

int32_t get_tick_count(){
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (int)tick;
}

void logger_put(const char* log){
    printf("%s\n", log);
}

int32_t sm_bp_node_id_select(int32_t _id){

}

int32_t sm_bp_node_id_deselect(int32_t _id){

}



sm_bp_node_id_controller_t g_node_id_controller = {
        .sm_bp_node_id_select = sm_bp_node_id_select,
        .sm_bp_node_id_deselect = sm_bp_node_id_deselect
};

static void sm_app_bp_service_run(void* _arg){
    sm_sv_bp_t* bp_service = (sm_sv_bp_t*)_arg;
    if(bp_service){
        sm_sv_bp_process(bp_service);
    }
}

static void sm_bp_auth_event_cb(int32_t _id, SM_BP_AUTH_EVENT _event, const char* _sn, int32_t _soc, void* _arg){
    LOG_INF(MAIN_TAG, "Assigning BP is %s, sn:%d, soc:%d", _event == BP_AUTH_SUCCESS ? "SUCCESS" : "FAILURE", _sn, _soc);
}

static void sm_app_auto_assigning_bp(void* _arg){
    sm_sv_bp_t* bp_service = (sm_sv_bp_t*)_arg;
    if(bp_service){
        if(!sm_sv_bp_is_authenticating(bp_service, BP_ID_DEFAULT)){
            sm_sv_bp_auth(bp_service, BP_ID_DEFAULT, sm_bp_auth_event_cb, NULL);
        }
/*        int32_t bp_numb = sm_sv_bp_get_number(bp_service);
        for(int index = 0; index < bp_numb; index++){
            if(!sm_sv_bp_is_authenticating(bp_service, index)){

            }
        }*/
    }
}

int main(int argc, char **argv){

    sm_sch_t* sch = sm_sch_create_default();

    sm_sv_bp_t* bp_service = sm_sv_bp_create(1, 0, &g_node_id_controller);

    sm_sch_start_task(sch, 0, SM_SCH_REPEAT_FOREVER, sm_app_bp_service_run, bp_service);
    sm_sch_start_task(sch, 8000, SM_SCH_REPEAT_FOREVER, sm_app_auto_assigning_bp, bp_service);
    while (1){
        sm_sch_process(sch);
    }
}