//
// Created by hainh on 8/12/2024.
//
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#define ALPHABET_SIZE 26
#define SCAN_INTERVAL 1000 // 1ms

#include "sm_sv_bt.h"
#include "string.h"

uint64_t key_st[256] = {0,};

int64_t get_tick_count(){
    struct timespec ts;
    unsigned int tick = 0U;
    clock_gettime( CLOCK_REALTIME, &ts );
    tick  = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return (int64_t )tick;
}

uint8_t button_0_get_state() {
    printf("key value: %ld\n",key_st[48]);
    if (key_st[48]) return 1;
    return 0;
}

uint8_t button_1_get_state() {
    printf("key value: %d\n",key_st[49]);
    if (key_st[49]) return 1;
    return 0;
}

void *sv_bt_thread(void *_arg) {
    while(1) {
        sm_sv_bt_process(_arg);
        usleep(10000);
    }
}

sm_sv_bt_t *g_sv_bt;
sm_bt_if btn_if[] = {button_0_get_state,button_1_get_state,};

void button_ev_cb(uint8_t _btn_id, uint8_t _event, void *_arg) {
    if (_event == SM_SV_BT_EVENT_TAP) {
        printf("BUTTON %d TAP \n", _btn_id);
    }else if (_event == SM_SV_BT_EVENT_HOLD) {
        printf("BUTTON %d HOLD \n", _btn_id);
    }
}

int is_key_pressed() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void main() {
    g_sv_bt = sm_sv_bt_create(btn_if, 1);
    sm_sv_bt_reg_event(g_sv_bt, 0, button_ev_cb, NULL);
    pthread_t pid;
    pthread_create(&pid, NULL, sv_bt_thread, g_sv_bt);
    char c;
    uint8_t refresh_cnt=0;
    while(1) {
        usleep(1000* 10);
        // for (int i=48; i <60; i++)
        //     printf("|key-%d: %d |",i,key_st[i]);
        // printf("\n");
        if (is_key_pressed()) {
            c = getchar();
            key_st[(uint8_t)c]++;
            continue;
        }
        if (refresh_cnt++ > 5) {
            refresh_cnt = 0;
            for (int i=48; i <60; i++)
                key_st[i] = 0;
        }
    }
}

