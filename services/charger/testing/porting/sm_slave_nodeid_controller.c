//
// Created by DELL 5425 on 4/17/2024.
//
#include "sm_bp_auth.h"
#include <stdint.h>
#include "stdlib.h"
#include "sm_string_t.h"
#include <stdio.h>
#include <unix/linux_serial.h>
#include "string.h"

#include <unistd.h>

#include "sm_slave_nodeid_controller.h"

#define USB0 "/dev/ttyUSB0"
int32_t g_fd;

int32_t sm_slave_node_id_select(int32_t _id){
    switch (_id) {
        case 0:
            serial_send_bytes(g_fd, ":0,W,N,1*", strlen(":0,W,N,1*"));
            break;
        case 1:
            serial_send_bytes(g_fd, ":1,W,N,1*", strlen(":1,W,N,1*"));
            break;
        case 2:
            serial_send_bytes(g_fd, ":2,W,N,1*", strlen(":2,W,N,1*"));
            break;
    }
    usleep(400*1000);
    return 0;
}

int32_t sm_slave_node_id_deselect(int32_t _id){
    switch (_id) {
        case 0:
            serial_send_bytes(g_fd, ":0,W,N,0*", strlen(":0,W,N,0*"));
            break;
        case 1:
            serial_send_bytes(g_fd, ":1,W,N,0*", strlen(":1,W,N,0*"));
            break;
        case 2:
            serial_send_bytes(g_fd, ":2,W,N,0*", strlen(":2,W,N,0*"));
            break;
    }
    usleep(400*1000);
    return 0;
}

static sm_bp_node_id_controller_t g_nodeid_controller = {
    .sm_bp_node_id_select = sm_slave_node_id_select,
    .sm_bp_node_id_deselect = sm_slave_node_id_deselect,
};


void* sm_slave_nodeid_controller_get(){
    g_fd = serial_init(USB0, 115200, SERIAL_FLAG_BLOCKING);
    if (g_fd<0) return NULL;
    printf("sm_slave_nodeid_controller_get OK \n");
    return &g_nodeid_controller;
}