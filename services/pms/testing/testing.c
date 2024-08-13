//
// Created by DELL 5425 on 3/22/2024.
//
#include "sm_sv_pms.h"
#include "sm_logger.h"
#include "sm_sv_charger.h"
#include "sm_bp_auth.h"
#include "sm_sv_bp.h"
#include "time.h"
#include <stdio.h>
#include <stdint.h>
//#include <ncurses.h>
#include <stdlib.h>
#include "sm_pms_est_data.h"
#include "porting/sm_slave_nodeid_controller.h"
#include <pthread.h>
#include <stddef.h>
#define MAINTAG "TEST.C"

typedef struct app {
    sm_sv_pms_t         *m_pms;
    sm_sv_bp_t          *m_bp;
    sm_sv_charger_t     *m_charger;
    sm_pms_data_t       *m_pms_tx;
    sm_pms_data_t       *m_pms_rx;
    est_data_t          *m_est_rx;
    sm_charger_data_t   *m_charger_tx;
    sm_charger_data_t   *m_charger_rx;
    uint8_t             is_charger_plugged_in;
    uint8_t             m_operartion_mode;
    uint8_t             is_bp_connected[3];
}testing_app;

testing_app p_app;
int8_t              m_event_num;
int8_t              m_event_list[10];

int8_t assigning_slave = 0;
int8_t is_assigning = -1;
uint8_t assign_times[3] = {0,0,0};

#define impl(x) ((testing_app*)(x))

int32_t g_charger_vol = 67000;
uint8_t use_log = 0;

const char* event_str[] = {
    "SM_SV_PMS_EVENT_IDLE",
    "SM_SV_PMS_EVENT_SWITCH_SUCCESS",
    "SM_SV_PMS_EVENT_SWITCH_FAIL",
    "SM_SV_PMS_EVENT_MERGE_SUCCESS",
    "SM_SV_PMS_EVENT_MERGE_FAIL",
};

const char* bp_st_str[] = {
        "BP_STATE_INIT ",
        "BP_STATE_IDLE",
        "BP_STATE_SOFT_START",
        "BP_STATE_DISCHARGING",
        "BP_STATE_CHARGING",
        "BP_STATE_FAULT",
        "BP_STATE_SHIP_MODE",
        "BP_STATE_SYSTEM_BOOST_UP",
        "BP_STATE_ID_ASSIGN_START",
        "BP_STATE_ID_ASSIGN_WAIT_CONFIRM",
        "BP_STATE_ID_ASSIGN_CONFIRMED",
        "BP_STATE_ID_ASSIGN_WAIT_SLAVE_SELECT",
        "BP_STATE_START_AUTHENTICATE",
        "BP_STATE_AUTHENTICATING",
        "BP_STATE_STANDBY",
        "BP_STATE_SHUTDOWN",
        "BP_STATE_ONLY_DISCHARGING"
};

void log_put(const char* _log){
//    if(strstr(_log, "SM_BP_AUTHEN") != NULL
//        || strstr(_log, "TEST.C") != NULL
//        || strstr(_log, "SM_SV_PMS") != NULL
//        )
    if (strstr(_log, "TEST.C") != NULL
    || strstr(_log, "SM_SV_PMS") != NULL
    )
    {
        printf("%s\n", _log);
    }
}
uint32_t get_tick_count(){
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (uint32_t )tick;
}

void (*switch_merge_cb_fn)(uint8_t , uint8_t , uint8_t , void*);
void (*update_data_cb_fn)(const sm_pms_data_t*, void*);
void (*update_est_data_cb_fn)(const est_data_t* , void*);

void pms_switch_merge_cb(uint8_t _impl_bp, uint8_t _next_bp, uint8_t _event, void* _arg){
    printf("PMS EVENT: %s , IMPL: %d - NEXT: %d\n", event_str[_event], _impl_bp, _next_bp);
    m_event_num += 1;
    m_event_list[m_event_num] = _event;
    printf("APP EVENT NUM : %d\n", m_event_num );
}
void pms_update_data_cb(const sm_pms_data_t* _pms_data, void* _arg){
//    pms_clone_data(&pms_rx_data_buff, _pms_data);
//    pms_clone_data(&pms_tx_data_buff, &pms_rx_data_buff);
    pms_clone_data(impl(_arg)->m_pms_rx, _pms_data);
    pms_clone_data(impl(_arg)->m_pms_tx, impl(_arg)->m_pms_rx);
//    pms_log_data(impl(_arg)->m_pms_tx);
}
void pms_update_est_data_cb(const est_data_t* _pms_est_data, void* _arg){
    est_data_cpy( _pms_est_data, impl(_arg)->m_est_rx);
//    printf("RANGE: %d \nCUR_MAX: %d\n", impl(_arg)->m_est_rx->m_distance_km, impl(_arg)->m_est_rx->m_cur_max);
}
sm_sv_pms_event_cb_fn_t pms_cb = {
        .switch_merge_cb_fn = pms_switch_merge_cb,
        .update_data_cb_fn = pms_update_data_cb,
        .update_est_data_cb_fn = pms_update_est_data_cb,
};

void charger_switch_merge_cb(uint8_t _impl_bp, uint8_t _next_bp, uint8_t _event, void* _arg){
    LOG_DBG(MAINTAG, "PMS EVENT: %d %s , IMPL: %d - NEXT: %d",_event, event_str[_event], _impl_bp, _next_bp);

}
void charger_update_data_cb(const sm_charger_data_t* _charger_data, void* _arg){
//    charger_clone_data(&charger_rx_data_buff, _charger_data);
//    charger_clone_data(&charger_tx_data_buff, &charger_rx_data_buff);
    charger_clone_data(impl(_arg)->m_charger_rx, _charger_data);
    charger_clone_data(impl(_arg)->m_charger_tx, impl(_arg)->m_charger_rx);
    charger_log_data(impl(_arg)->m_charger_tx);
}
void charger_plugged_in_cb(uint8_t _level, void* _arg){
    impl(_arg)->is_charger_plugged_in = _level;
}
sm_sv_charger_event_cb_fn_t charger_cb = {
        .switch_merge_cb_fn = charger_switch_merge_cb,
        .update_data_cb_fn = charger_update_data_cb,
        .charger_plugged_in_cb_fn = charger_plugged_in_cb,
};

int32_t node_id_select(int32_t _id){
    LOG_DBG("MAIN", "NODE-ID %d : SELECT\n",_id);
    return 0;
}
int32_t node_id_deselect(int32_t _id){
    LOG_DBG("MAIN", "NODE-ID %d : DESELECT\n",_id);
    return 0;
}

static void bp_connected_cb_fn(int32_t _id, const char* _sn, int32_t _soc, void* _arg){
    LOG_DBG(MAINTAG, "BP %d soc %d sn %s connected", _id, _soc, _sn);
    pms_bp_connected_cb_fn(_id, _sn, _soc, p_app.m_pms);
    p_app.is_bp_connected[_id] = 1;
    is_assigning = -1;
    assign_times[_id] = 0;
}
static void bp_disconnected_cb_fn(int32_t _id, const char* _sn, void* _arg){
    LOG_DBG(MAINTAG, "BP %d sn %s disconnected", _id, _sn);
    pms_bp_disconnected_cb_fn( _id, _sn, p_app.m_pms);
    p_app.is_bp_connected[_id] = 0;
}
static void bp_update_data_cb_fn(int32_t _id, const sm_bp_data_t* _bp_data, void* _arg){
//    LOG_DBG(MAINTAG,"Receive BP data %d : %s %d %d", _id, bp_st_str[_bp_data->m_state], _bp_data->m_soc, _bp_data->m_vol);
    pms_bp_update_data_cb_fn(_id, _bp_data, p_app.m_pms);
}

sm_sv_bp_event_cb_t bp_cb = {
        .on_bp_connected = bp_connected_cb_fn,
        .on_bp_disconnected = bp_disconnected_cb_fn,
        .on_bp_update_data = bp_update_data_cb_fn,
};

FILE *f_test_db;
int32_t db[100][100];

enum {
    BP_NUM=0,
    SOC1,
    VOL1,
    SOC2,
    VOL2,
    SOC3,
    VOL3,
    ON_CHARGER,
    EVENT_NUM,
    EVENT1,
    EVENT2,
};
uint32_t case_num = 0;
uint32_t case_target = 0;

uint8_t bpnum = 0;
uint32_t pms_timer = 100;
uint32_t charger_st_timer = 200 ;
uint32_t is_charger_on = 0;
uint8_t end_flag = 0;

void* view_model_thread(void* _arg){
    char line[10];
    int count = 0;
    while(1){
        memset(line,'\0',10);
        count = 0;
        printf("<d>_<port>_<is_enable> : DISABLE OR ENABLE A PORT\n");
        printf("<f>_<port A>_<port B>  : FORCE SWITCH PORT A TO PORT B\n");
        printf("<f>_<port A>_<m>       : FORCE MERGE PORT A \n");
        printf("<r>_<port A>_<r>       : RELEASE PORT A \n");

        while (count < 10) {
            int c = fgetc(stdin);
//            int c = getchar();
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
        }
        printf("CMD: %s\n", line);
        switch (line[0]) {
            case 'd':
//                sm_sv_pms_switch_merge_enable(_arg, line[2]-48, line[4]-48);
                break;
            case 'f':
                switch (line[4]) {
                    case 'm':
//                        sm_sv_pms_force_merge(_arg, line[2]-48);
                        break;
                    case 'r':
//                        sm_sv_pms_release_switch_merge(_arg, line[2]-48);
                        break;
                    default:
//                        sm_sv_pms_force_switch(_arg, line[2]-48);
                        break;
                }
                break;
            default:
                break;
        }
    }
}

int main(){

    sm_logger_init(log_put, LOG_LEVEL_DEBUG);
    {
        sm_pms_data_t* pms_rx_data_buff = malloc(sizeof (sm_pms_data_t));
        sm_pms_data_t* pms_tx_data_buff = malloc(sizeof (sm_pms_data_t));

        est_data_t* pms_est_data_rx_buff = malloc(sizeof (est_data_t));

        pms_data_reset(pms_tx_data_buff);
        pms_data_reset(pms_rx_data_buff);

        sm_sv_bp_t *g_bp = sm_sv_bp_create(3, 0, sm_slave_nodeid_controller_get());
        sm_sv_pms_t *g_pms = sm_sv_pms_create(g_bp);

        sm_sv_pms_reg_event(g_pms, &pms_cb, &p_app);
        sm_sv_bp_reg_event(g_bp, &bp_cb, NULL);

        p_app.m_pms = g_pms;
        p_app.m_bp  = g_bp;
        p_app.m_pms_tx = pms_tx_data_buff;
        p_app.m_pms_rx = pms_rx_data_buff;
        p_app.m_est_rx = pms_est_data_rx_buff;

        p_app.is_bp_connected[0] = 0;
        p_app.is_bp_connected[1] = 0;
        p_app.is_bp_connected[2] = 0;

        uint64_t sys_time, start_time;
        start_time = get_tick_count();
        sys_time = get_tick_count();
        uint64_t reassign_timeout = 0;
        uint8_t is_enable = 1;
        sm_sv_bp_process(p_app.m_bp);
//        sm_sv_bp_auth(p_app.m_bp, 2, NULL, NULL);

//        pthread_t vm_pid;
//        pthread_create(&vm_pid, NULL, view_model_thread, p_app.m_pms);

        while(1){
            sm_sv_bp_process(p_app.m_bp);

            if (get_tick_count() - start_time > 200){
                sm_sv_pms_process(p_app.m_pms);
                start_time = get_tick_count();
            }
            if (get_tick_count() - sys_time > 25000){

//                sm_sv_pms_switch_merge_enable(p_app.m_pms, 0, is_enable++%2);
                if (is_enable)
                {
                    LOG_DBG(MAINTAG, "FORCE SWITCH BP REQ");
                    sm_sv_pms_force_switch(p_app.m_pms, 2);
                    is_enable = 0;
                }
                else
                {
                    LOG_DBG(MAINTAG, "RELEASE SWITCH BP REQ");
                    sm_sv_pms_release_switch(p_app.m_pms, 2);
                    is_enable = 1;
                }
                sys_time = get_tick_count();

            }

            if (get_tick_count() - reassign_timeout > 2000){
                if (assigning_slave > 2)
                {
                    assigning_slave = 0;
                }
                if (p_app.is_bp_connected[assigning_slave] == 0)
                {
                    if (assign_times[assigning_slave] > 0) {
                        assign_times[assigning_slave] = 0;
                        assigning_slave++;
                        is_assigning = -1;
                        continue;
                    }
                    if (is_assigning != -1) continue;
                    is_assigning = assigning_slave;
                    sm_sv_bp_auth(p_app.m_bp, assigning_slave, NULL, NULL);
                    assign_times[assigning_slave]++;
//                    LOG_DBG(MAINTAG, "ASSIGNING SLAVE: %d", assigning_slave);
                }
                else{
                    assigning_slave++;
                }

                reassign_timeout = get_tick_count();
            }
        }

    }

}

