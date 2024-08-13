//
// Created by DELL 5425 on 3/13/2024.
//
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "sm_logger.h"
#include "sm_sv_charger.h"
#include "sm_bp_auth.h"
#include "sm_sv_charger.h"
#include "porting/sm_slave_nodeid_controller.h"
#include "pthread.h"
static const char* TAG = "TEST";
typedef enum {
    SM_SLAVE_DISCONNECTED,
    SM_SLAVE_AUTHORIZING,
    SM_SLAVE_CONNECTED,
} SLAVE_CONNECTION_ST;

uint32_t CH_VOL = 0;

void log_put(const char* _log){
    if (strstr(_log, "TEST") != NULL ||
            strstr(_log, "SV_CHARGER") != NULL
//            strstr(_log, "SM_SV_BP") != NULL
            )
        printf("%s\n", _log);
}

int32_t get_tick_count(){
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (int32_t )tick;
}
int32_t charger_get_vol(){
    return CH_VOL;
}
sm_sv_charger_if_t g_charger_if = {
        .get_charger_vol_fn_t = charger_get_vol
};
sm_sv_charger_prof_t g_charger_prof = {
        .m_max_cur = 12000,
        .m_max_temp = 100,
        .m_max_volt = 67000,
        .m_min_volt = 54000,
        .m_power_stable_time = 3000,
};
typedef struct app {
    sm_sv_bp_t          *m_bp;
    sm_sv_charger_t     *m_charger;
    uint8_t             is_bp_connected[3];
    elapsed_timer_t     m_assign_timeout;
    int8_t              m_slave[3];
    int8_t              m_assigning_slave;
}testing_app;

#define impl(x) ((testing_app*)(x))

uint64_t reassign_timeout = 0;
int8_t assigning_slave;

void sm_sv_on_bp_connected(int32_t _id, const char* _sn, int32_t _soc, void* _arg){
    LOG_DBG(TAG, "BP %d CONNECTED soc %d", _id, _soc);
    testing_app *app = (testing_app*)_arg;
    app->m_slave[_id] = SM_SLAVE_CONNECTED;
    sm_sv_charger_on_bp_connected(_id, _sn, _soc, (void*) app->m_charger);
}
void sm_sv_on_bp_disconnected(int32_t _id, const char* _sn, void* _arg){
    LOG_DBG(TAG, "BP %d DISCONNECTED", _id);
    testing_app *app = (testing_app*)_arg;
    app->m_slave[_id] = SM_SLAVE_DISCONNECTED;
    sm_sv_charger_on_bp_disconnected( _id, _sn, (void*) app->m_charger);
}
sm_sv_bp_event_cb_t g_bp_event_cb = {
        .on_bp_connected = sm_sv_on_bp_connected,
        .on_bp_disconnected = sm_sv_on_bp_disconnected,
        .on_bp_update_data = NULL,
};

void sm_app_on_plugged_in(uint8_t _event, void* _arg){
    if (_event)
        LOG_DBG(TAG, "CHARGER PLUGGED");
    else
        LOG_DBG(TAG, "CHARGER UNPLUGGED");
}

void sm_app_on_charged(void* _arg){
    LOG_DBG(TAG, "ON CHARGE");
}
sm_sv_charger_event_cb_fn_t g_chg_event_cb = {
        .on_plugged_in = sm_app_on_plugged_in,
        .on_charged     = sm_app_on_charged,
        .on_error       = NULL,
        .on_update_data = NULL,
};

char ch;
int32_t err = 0;
void *test_thread(void* _arg){
    while (1) {
        printf(" 1 : Force to BP0\n");
        printf(" 2 : Force to BP1\n");
        printf(" 3 : Force to BP2\n");
        printf(" 4 : Release \n");
        printf(" q : ON Charger\n");
        printf( " w : OFF Charge\n");
        printf(" a : Increase diff vol 500mV\n");
        printf( " z : Decrease diff vol 500mV\n");
        printf(" x : Disable BP 1\n");
        printf(" c : Disable BP 2\n");
        printf(" v : Disable BP 3\n");
        printf(" s : Enable BP 1\n");
        printf(" d : Enable BP 2\n");
        printf(" f : Enable BP 3\n");
        fflush(stdin);
        scanf("%c", &ch);
        switch (ch) {
            case '1':
                err = sm_sv_charger_force(impl(_arg)->m_charger, 0);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case '2':
                err = sm_sv_charger_force(impl(_arg)->m_charger, 1);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case '3':
                err = sm_sv_charger_force(impl(_arg)->m_charger, 2);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case '4':
                err = sm_sv_charger_release(impl(_arg)->m_charger);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case 'q':
                CH_VOL = 66000;
                break;
            case 'w':
                CH_VOL = 0;
                break;
            case 'a':
                inc_vol_diff_500mV();
               break;
            case 'z':
                dec_vol_diff_500mV();
                break;
            case 'x':
                err = sm_sv_charger_disable_bp(impl(_arg)->m_charger, 0);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case 'c':
                err = sm_sv_charger_disable_bp(impl(_arg)->m_charger, 1);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case 'v':
                err = sm_sv_charger_disable_bp(impl(_arg)->m_charger, 2);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case 's':
                err = sm_sv_charger_enable_bp(impl(_arg)->m_charger, 0);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case 'd':
                err = sm_sv_charger_enable_bp(impl(_arg)->m_charger, 1);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
            case 'f':
                err = sm_sv_charger_enable_bp(impl(_arg)->m_charger, 2);
                if (err)
                    LOG_ERR(TAG, "ERR %d: ", err);
                break;
        }
        ch = ' ';
    }
}
testing_app p_app;
void main(){
    sm_logger_init(log_put, LOG_LEVEL_DEBUG);
    sm_sv_bp_t* g_bp = sm_sv_bp_create(3, 0, sm_slave_nodeid_controller_get());;
    sm_sv_charger_t* g_charger = sm_sv_charger_create_default(&g_charger_prof, &g_charger_if, g_bp);
    sm_sv_charger_reg_event(g_charger, &g_chg_event_cb, &p_app);
    sm_sv_bp_reg_event(g_bp, &g_bp_event_cb,&p_app);
    p_app.m_bp = g_bp;
    p_app.m_charger = g_charger;
    elapsed_timer_resetz(&p_app.m_assign_timeout, 500);
    p_app.m_assigning_slave = 0;
    for (int i = 0; i < 3; ++i) {
        p_app.m_slave[i] = SM_SLAVE_DISCONNECTED;
    }

    elapsed_timer_t charger_tm;
    elapsed_timer_resetz(&charger_tm, 0);

    pthread_t p_id;
    pthread_create(&p_id, NULL, &test_thread, &p_app);
    while(1){
        sm_sv_bp_process(g_bp);

        if(!elapsed_timer_get_remain(&charger_tm)){
            sm_sv_charger_process(g_charger);
            elapsed_timer_reset(&charger_tm);
        }

        if (!elapsed_timer_get_remain(&p_app.m_assign_timeout)){
            switch (p_app.m_slave[p_app.m_assigning_slave]){
                case SM_SLAVE_DISCONNECTED:
                    p_app.m_slave[p_app.m_assigning_slave] = SM_SLAVE_AUTHORIZING;
                    sm_sv_bp_auth(p_app.m_bp, p_app.m_assigning_slave, NULL, NULL);
//                    LOG_DBG(TAG, "ASSIGNING SLAVE %d", p_app.m_assigning_slave);
                    break;
                case SM_SLAVE_AUTHORIZING:

                    p_app.m_slave[p_app.m_assigning_slave] = SM_SLAVE_DISCONNECTED;
                    if (++p_app.m_assigning_slave == 3)
                        p_app.m_assigning_slave = 0;
                    break;
                case SM_SLAVE_CONNECTED:
                    if (++p_app.m_assigning_slave == 3)
                        p_app.m_assigning_slave = 0;
                    break;
            }
            elapsed_timer_reset(&p_app.m_assign_timeout);
        }
    }

}