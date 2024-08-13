#include <stdio.h>
#include "sm_bsp_win.h"
#include "sm_sv_io.h"
#include "stdint.h"
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "stdlib.h"
#include <ncurses.h>
#include "sm_logger.h"

void log_put(const char* _log){
    printf("%s\n", _log);
}
#define setcur(x,y) printf ("\033[%d;%dH",x,y) // x-row y-col
FILE* file_log;

extern uint8_t port_pin_value[16][16];
extern uint8_t port_pin_mode[16][16];

static char buffer[100];

void pmu_io_sv_event_cb(uint8_t _event, void* _arg);
void reverse_state(uint8_t *_state);
void ev_emulator();

const char *event_str[] = {
        "SM_SV_IO_EVENT_IN_KEY_CHANGED_ON",
        "SM_SV_IO_EVENT_IN_KEY_CHANGED_OFF",

        "SM_SV_IO_EVENT_IN_REVERSE_CHANGE_ON",
        "SM_SV_IO_EVENT_IN_REVERSE_CHANGE_OFF",

        "SM_SV_IO_EVENT_IN_LEFT_CHANGED_ON",
        "SM_SV_IO_EVENT_IN_LEFT_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_LEFT_CHANGED_ON",
        "SM_SV_IO_EVENT_OUT_LEFT_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_LEFT_CHANGED_BLINK",

        "SM_SV_IO_EVENT_IN_RIGHT_CHANGED_ON",
        "SM_SV_IO_EVENT_IN_RIGHT_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_ON",
        "SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_RIGHT_CHANGED_BLINK",

        "SM_SV_IO_EVENT_IN_PHARE_CHANGED_ON",
        "SM_SV_IO_EVENT_IN_PHASE_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_PHASE_CHANGED_ON",
        "SM_SV_IO_EVENT_OUT_PHASE_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_PHASE_CHANGED_BLINK",

        "SM_SV_IO_EVENT_IN_HORN_CHANGED_ON",
        "SM_SV_IO_EVENT_IN_HORN_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_HORN_CHANGED_ON",
        "SM_SV_IO_EVENT_OUT_HORN_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_HORN_CHANGED_BLINK",

        "SM_SV_IO_EVENT_IN_COS_CHANGED_ON",
        "SM_SV_IO_EVENT_IN_COS_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_COS_CHANGED_ON",
        "SM_SV_IO_EVENT_OUT_COS_CHANGED_OFF",
        "SM_SV_IO_EVENT_OUT_COS_CHANGED_BLINK",

        "SM_SV_IO_EVENT_IN_BREAK_CHANGE_ON",
        "SM_SV_IO_EVENT_IN_BREAK_CHANGE_OFF",

        "SM_SV_IO_EVENT_OUT_TAIL_LIGHT_CHANGE_ON",
        "SM_SV_IO_EVENT_OUT_TAIL_LIGHT_CHANGE_OFF",
        "SM_SV_IO_EVENT_IN_PARKING_PRESSED",
        "SM_SV_IO_EVENT_IN_DRV_MODE_PRESSED",
};
sm_sv_io_if_t sm_bsp_if = {
        .in_key_fn_t            = sm_bsp_io_get_value_key,
        .in_left_light_fn_t     = sm_bsp_io_get_value_left_light,
        .in_right_light_fn_t    = sm_bsp_io_get_value_right_light,
        .in_cos_light_fn_t      = sm_bsp_io_get_value_cos_light,
        .in_phase_light_fn_t    = sm_bsp_io_get_value_phare_light,
        .in_driving_mode_fn_t   = sm_bsp_io_get_value_driving_mode,
        .in_parking_fn_t        = sm_bsp_io_get_value_parking,
        .in_left_break_fn_t     = sm_bsp_io_get_value_left_break,
        .in_right_break_fn_t    = sm_bsp_io_get_value_right_break,
        .in_reverse_fn_t        = sm_bsp_io_get_value_reverse_mode,
        .in_horn_fn_t           = sm_bsp_io_get_value_horn,
        .out_left_light_fn_t    = (void*)sm_bsp_io_set_value_left_light,
        .out_right_light_fn_t   = (void*)sm_bsp_io_set_value_right_light,
        .out_code_light_fn_t    = (void*)sm_bsp_io_set_value_cos_light,
        .out_phase_light_fn_t   = (void*)sm_bsp_io_set_value_phare_light,
        .out_horn_fn_t          = (void*)sm_bsp_io_set_value_horn,
        .out_tail_light_fn_t    = (void*)sm_bsp_io_set_value_tail_light
};


void *sv_io_thread(void *_arg){
    initscr();
    curs_set(0);
    int32_t timer1;
    int32_t timer2;
    while(1){
        if (get_tick_count() - timer1 > 10)
        {
            sm_sv_io_process(_arg);
            timer1 = get_tick_count();
            ev_emulator();
        }
        if (get_tick_count() - timer2 > 200){
            refresh();
            timer2 = get_tick_count();
        }
    }
}

int main(){
    int32_t err;
    initscr();
    curs_set(1);
//    sm_logger_init(log_put, LOG_LEVEL_DEBUG);
    sm_bsp_init();
    sm_sv_io_t *pmu_io_sv = sm_sv_io_create(&sm_bsp_if);
    sm_sv_io_set_if(pmu_io_sv, &sm_bsp_if);
    sm_sv_io_reg_event(pmu_io_sv, pmu_io_sv_event_cb, pmu_io_sv);

    char ch;
    uint8_t id=0;
    uint8_t parking_cnt =0;
    uint8_t drv_mode = 0;

    mvprintw(0,0,"\
.__________________________________________________________________________________________________.\n\
|   IO   |   Key   |   Left  |  Right  |  Phase  |   Cos   |   Park  |   Drv   |   Rev   |   Horn  |\n\
|   ID   |    0    |    1    |    2    |    3    |    4    |    5    |    6    |    7    |    8    |\n\
|   IN   |         |         |         |         |         |         |         |         |         |\n\
|___OUT__|_________|_________|_________|_________|_________|_________|_________|_________|_________|");

    mvprintw(12,10,"  q : set LEFT light OFF           d : release PHASE\n\
            w : set LEFT light ON            f : release COS\n\
            e : set RIGHT light OFF          g : set HORN OFF\n\
            r : set RIGHT light ON           h : set HORN ON\n\
            a : release LEFT light           j : release HORN\n\
            s : release RIGHT light          k : blink HORN\n\
            t : set PHASE OFF\n\
            y : set PHASE ON\n\
            u : set COS OFF\n\
            i : set COS ON");

    sleep(1);
    pthread_t sv_io_pid;
    pthread_create(&sv_io_pid, NULL, sv_io_thread, pmu_io_sv);
    while (1){
        if ( 1 ) {
            ch = getchar();
            id = ch - 48;
            switch (ch) {
                case 'q':
                    sm_sv_io_set_output_left_light(pmu_io_sv, 0);
                    break;
                case 'w':
                    sm_sv_io_set_output_left_light(pmu_io_sv, 1);
                    break;
                case 'e':
                    sm_sv_io_set_output_right_light(pmu_io_sv, 0);
                    break;
                case 'r':
                    sm_sv_io_set_output_right_light(pmu_io_sv, 1);
                    break;
                case 'a':
                    sm_sv_io_release_output_left_light(pmu_io_sv);
                    break;
                case 's':
                    sm_sv_io_release_output_right_light(pmu_io_sv);
                    break;
                case 't':
                    sm_sv_io_set_output_phare_light(pmu_io_sv,0);
                    break;
                case 'y':
                    sm_sv_io_set_output_phare_light(pmu_io_sv,1);
                    break;
                case 'u':
                    sm_sv_io_set_output_code_light(pmu_io_sv,0);
                    break;
                case 'i':
                    sm_sv_io_set_output_code_light(pmu_io_sv,1);
                    break;
                case 'd':
                    sm_sv_io_release_output_phare_light(pmu_io_sv);
                    break;
                case 'f':
                    sm_sv_io_release_output_code_light(pmu_io_sv);
                    break;
                case 'g':
                    sm_sv_io_set_output_horn(pmu_io_sv,0);
                    break;
                case 'h':
                    sm_sv_io_set_output_horn(pmu_io_sv,1);
                    break;
                case 'j':
                    sm_sv_io_release_output_horn(pmu_io_sv);
                    break;
                case 'k':
                    sm_sv_io_set_horn_blink(pmu_io_sv, 500, 1000);
                    break;
                case 'l':
                    sm_sv_io_release_output_horn_blink(pmu_io_sv);
                    break;
            }
            if (id == 5) {
                if (parking_cnt++ > 5)
                {
                    reverse_state(&port_pin_value[0][5]);
                    parking_cnt = 0;
                }
            }
            else if (id == 6) {
                if (drv_mode++ > 5)
                {
                    reverse_state(&port_pin_value[0][6]);
                    drv_mode = 0;
                }
            }
            else{
                reverse_state(&port_pin_value[0][id]);
                parking_cnt =0;
                drv_mode = 0;
            }
        }
    }
}

void pmu_io_sv_event_cb(uint8_t _event, void* _arg){
    (void*) _arg;
    mvprintw(10,10,"                                                                                  ");
    mvprintw(10,10,"EVENT: %s\n",event_str[_event]);
}

void reverse_state(uint8_t *_state){
    if (*_state == 0) *_state = 1;
    else *_state = 0;
}

void ev_emulator(){
    for(uint8_t i=0; i<9; i++){
        mvprintw(3,10+5+10*i-1,"%d", port_pin_value[0][i]);
    }
    if (port_pin_value[0][11] == 1) {
        mvprintw(4,10+5+10*1-2,"###");
    }
    else {
        mvprintw(4,10+5+10*1-2,"___");
    }
    if (port_pin_value[0][12] == 1) {
        mvprintw(4,10+5+10*2-2,"###");
    }
    else {
        mvprintw(4,10+5+10*2-2, "___");
    }
    mvprintw(4,10+5+10*3-1,"%d", port_pin_value[0][13]);
    mvprintw(4,10+5+10*4-1,"%d", port_pin_value[0][14]);
    mvprintw(4,10+5+10*8-1,"%d", port_pin_value[0][15]);
}
