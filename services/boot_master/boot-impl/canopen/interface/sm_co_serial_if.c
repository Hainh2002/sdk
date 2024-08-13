//
// Created by vnbk on 07/04/2023.
//
#include <canopen/interface/sm_co_if.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "sm_host.h"
#include "linux/utils/linux_serial.h"

#define PACKET_LENGTH   64

#define FORWARD_CANOPEN 0x20

struct sm_co_if{
    const char* m_serial_port;
    int32_t m_baud;
    int32_t m_fd;
    sm_host_t* m_serial_host;
    sm_co_if_receive_callback_fn m_callback;
};

static sm_co_if_t g_canopen_if = {
        .m_serial_port = NULL,
        .m_baud = 115200,
        .m_fd = -1,
        .m_serial_host = NULL,
        .m_callback = NULL,
};

int32_t sm_canopen_serial_send(const uint8_t* data, int32_t _len){
    int32_t len = serial_send_bytes(g_canopen_if.m_fd, (uint8_t*)data, _len);
    if(len <= 0){
        printf("Error Serial send: FD: %d\n", g_canopen_if.m_fd);
    }
    return len;
}

int32_t sm_canopen_host_cmd_callback(int32_t cmd, const uint8_t* data, int32_t len, void* arg){
    sm_co_if_t* canopen_if = (sm_co_if_t*)(arg);
    if(canopen_if->m_callback){
        uint16_t frame_id = ((uint16_t)data[1] << 8) | (uint16_t)data[2];
        canopen_if->m_callback(frame_id, (uint8_t*)&data[3]);

//        if(frame_id>=0x500){
//            printf("FrameId: 0x%2X\n", frame_id);
//        }
    }
    return 0;
}

sm_co_if_t *sm_co_if_create(uint8_t _type, const char *host, int port, void* _arg){
    return NULL;
}

sm_co_if_t *sm_co_if_create_default(uint8_t _type, const char *host, int port, void* _arg){
    if(_type == CO_SERIAL_CANBUS_IF) {
        g_canopen_if.m_serial_port = host;
        g_canopen_if.m_baud = port;
        g_canopen_if.m_fd = -1;
        g_canopen_if.m_serial_host = sm_host_create_default(SM_HOST_ADDR_DEFAULT, sm_canopen_serial_send);
        sm_host_reg_handle(g_canopen_if.m_serial_host, sm_canopen_host_cmd_callback, &g_canopen_if);

        return &g_canopen_if;
    }
    return NULL;
}

int sm_co_if_free(sm_co_if_t* self){
    if(!self){
        return -1;
    }
    if(serial_is_open(self->m_fd)){
        serial_free(self->m_fd);
        self->m_serial_port = NULL;
        self->m_fd = -1;
    }
    if(self->m_serial_host){
        sm_host_destroy(self->m_serial_host);
    }
    return 0;
}

int sm_co_if_set_config(sm_co_if_t *self, const char *_argv, int _argc, void* _arg){
    if(!self){
        return -1;
    }
    self->m_serial_port = _argv;
    self->m_baud = _argc;

    return 0;
}

int sm_co_if_reg_recv_callback(sm_co_if_t* self, sm_co_if_receive_callback_fn callback_fn){
    if(!self){
        return -1;
    }
    self->m_callback = callback_fn;
    return 0;
}

int sm_co_if_connect(sm_co_if_t *self){
    return 0;
}

int sm_co_if_disconnect(sm_co_if_t *self){
    return 0;
}

int sm_co_if_is_connected(sm_co_if_t *self){
    return 0;
}

int sm_co_if_send(sm_co_if_t *self, uint32_t frame_id, const unsigned char *data, int len, int timeout){
    if(!self || !serial_is_open(self->m_fd)){
        return -1;
    }
    uint8_t buf[PACKET_LENGTH] = {0,};
    int index = 0;
    frame_id &= 0xFFFF;

    buf[index++] = (frame_id >> 8) & 0xFF;
    buf[index++] = frame_id & 0xFF;
    for(int i = 0; i < len; i++){
        buf[index++] = data[i];
    }

    /*if(frame_id>=0x500){
        printf("Send CanOpen FrameId: 0x%2X\n", frame_id);
    }*/

    return sm_host_send_cmd(self->m_serial_host, FORWARD_CANOPEN, buf, index);
}

int sm_co_if_recv(sm_co_if_t *self, unsigned char *buf, int max_len, int timeout){

    return 0;
}

int sm_co_if_process(sm_co_if_t *self){
    if(!self) {
        return -1;
    }
    if(!serial_is_open(self->m_fd)){
        int fd = serial_init(self->m_serial_port, self->m_baud, !SERIAL_FLAG_BLOCKING);
        if(fd < 0){
            usleep(1000*1000);
            return 0;
        }
        self->m_fd = fd;
    }

    uint8_t buf[PACKET_LENGTH] = {0, };
    int len = serial_recv_bytes(self->m_fd, buf, PACKET_LENGTH);
    if(len > 0){
        sm_host_asyn_feed(buf, len, self->m_serial_host);
    }
    sm_host_process(self->m_serial_host);

    return 0;
}