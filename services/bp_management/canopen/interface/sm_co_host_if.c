//
// Created by vnbk on 07/04/2023.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "sm_co_if.h"
#include "sm_host.h"
#include "linux/utils/linux_serial.h"

#define PACKET_LENGTH   64

struct sm_co_if{
    const char* m_serial_port;
    int32_t m_baud;
    int32_t m_fd;
    sm_host_t* m_serial_host;
    sm_co_if_recv_callback_fn_t m_callback;
};

static sm_co_if_t g_co_if = {
        .m_serial_port = NULL,
        .m_baud = 115200,
        .m_fd = -1,
        .m_serial_host = NULL,
        .m_callback = NULL,
};

static int32_t sm_co_host_serial_send_if(const uint8_t* _data, int32_t _len, int32_t _timeout, void* _arg){
    (void)_timeout;
    sm_co_if_t* co_if = (sm_co_if_t*)(_arg);

//    printf("send %d byte to master via can if with cmd %d\n",_len, _data[4]);

    if(sm_host_get_mode(co_if->m_serial_host) == SM_HOST_SYNC_MODE){
        serial_flush(co_if->m_fd);
    }
    int32_t len = serial_send_bytes(co_if->m_fd, (uint8_t*)_data, _len);
    if(len <= 0){
        printf("Error Serial send: FD: %d\n", co_if->m_fd);
    }
    return len;
}

int32_t sm_co_host_serial_recv_if(uint8_t *_buf, int32_t _max_len, int32_t _timeout, void* _arg) {
    uint32_t poll_timeout = get_tick_count() + _timeout;
    int32_t len = 0;
    int ret = 0;
    sm_co_if_t* co_if = (sm_co_if_t*)(_arg);

    while (poll_timeout >= get_tick_count() && len < _max_len) {
        ret = serial_recv_bytes(co_if->m_fd, _buf, _max_len);
        if (ret >= 0) {
            len += ret;
        }
    }
    return len;
}


static int32_t sm_co_host_cmd_callback(int32_t cmd, const uint8_t* data, int32_t len, void* arg){
    sm_co_if_t* co_if = (sm_co_if_t*)(arg);
    if(co_if->m_callback/* && cmd == SM_CMD_CANOPEN_FORWARD*/){
        uint16_t frame_id = ((uint16_t)data[1] << 8) | (uint16_t)data[2];
        co_if->m_callback(frame_id, (uint8_t*)&data[3]);
    }
    return 0;
}

sm_co_if_t *sm_co_if_create(uint8_t _type, const char *host, int port, void* _arg){
    if(_type == CO_SERIAL_CANBUS_IF) {
        sm_co_if_t* co_if = (sm_co_if_t*)malloc(sizeof(sm_co_if_t));
        if(!co_if){
            return NULL;
        }
        co_if->m_serial_port = host;
        co_if->m_baud = port;
        co_if->m_fd = -1;
        co_if->m_serial_host = (sm_host_t*)_arg;

        sm_host_set_addr(co_if->m_serial_host, SM_HOST_ADDR_DEFAULT);
        sm_host_set_mode(co_if->m_serial_host, SM_HOST_ASYNC_MODE);
        sm_host_set_host_if(co_if->m_serial_host, sm_co_host_serial_send_if, sm_co_host_serial_recv_if, co_if);

        sm_host_reg_handle(co_if->m_serial_host, sm_co_host_cmd_callback, &g_co_if);

        return co_if;
    }
    return NULL;
}

sm_co_if_t *sm_co_if_create_default(uint8_t _type, const char *host, int port, void* _arg){
    if(_type == CO_SERIAL_CANBUS_IF) {
        g_co_if.m_serial_port = host;
        g_co_if.m_baud = port;
        g_co_if.m_fd = -1;

        g_co_if.m_serial_host = sm_host_create_default(SM_HOST_ASYNC_MODE, 1, NULL, NULL, NULL);

        sm_host_set_addr(g_co_if.m_serial_host, SM_HOST_ADDR_DEFAULT);
        sm_host_set_mode(g_co_if.m_serial_host, SM_HOST_ASYNC_MODE);
        sm_host_set_host_if(g_co_if.m_serial_host, sm_co_host_serial_send_if, sm_co_host_serial_recv_if, &g_co_if);

        sm_host_reg_handle(g_co_if.m_serial_host, sm_co_host_cmd_callback, &g_co_if);

        return &g_co_if;
    }
    return NULL;
}

int sm_co_if_free(sm_co_if_t* _this){
    if(!_this){
        return -1;
    }
    if(serial_is_open(_this->m_fd)){
        serial_free(_this->m_fd);
        _this->m_serial_port = NULL;
        _this->m_fd = -1;
    }
    if(_this->m_serial_host){
        sm_host_destroy(_this->m_serial_host);
    }
    return 0;
}

int sm_co_if_set_config(sm_co_if_t *_this, const char *_argv, int _argc, void* _arg){
    if(!_this){
        return -1;
    }
    _this->m_serial_port = _argv;
    _this->m_baud = _argc;

    return 0;
}

int sm_co_if_reg_recv_callback(sm_co_if_t* _this, sm_co_if_recv_callback_fn_t callback_fn){
    if(!_this){
        return -1;
    }
    _this->m_callback = callback_fn;
    return 0;
}

void* sm_co_if_get_interface(sm_co_if_t* _this){
    if(!_this){
        return NULL;
    }
    return _this->m_serial_host;
}

int sm_co_if_connect(sm_co_if_t *_this){
    if(!serial_is_open(_this->m_fd)){
        int fd = serial_init(_this->m_serial_port, _this->m_baud, !SERIAL_FLAG_BLOCKING);
        if(fd < 0){
            usleep(1000*1000);
            return -1;
        }
        _this->m_fd = fd;
    }
    return 0;
}

int sm_co_if_disconnect(sm_co_if_t *_this){
    return serial_free(_this->m_fd);
}

int sm_co_if_is_connected(sm_co_if_t *_this){
    return serial_is_open(_this->m_fd);
}

int sm_co_if_send(sm_co_if_t *_this, uint32_t frame_id, const unsigned char *data, int len, int timeout){
    if(!_this || !serial_is_open(_this->m_fd)){
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


    return sm_host_send_cmd(_this->m_serial_host, SM_CMD_CANOPEN_FORWARD, buf, index);
}

int sm_co_if_recv(sm_co_if_t *_this, unsigned char *buf, int max_len, int timeout){
    return 0;
}

int sm_co_if_process(sm_co_if_t *_this){
    if(!_this) {
        return -1;
    }
    if(!serial_is_open(_this->m_fd)){
        int fd = serial_init(_this->m_serial_port, _this->m_baud, !SERIAL_FLAG_BLOCKING);
        if(fd < 0){
            usleep(1000*1000);
            return 0;
        }
        _this->m_fd = fd;
    }

    uint8_t buf[PACKET_LENGTH] = {0, };
    int len = serial_recv_bytes(_this->m_fd, buf, PACKET_LENGTH);
    if(len > 0)
    {
        sm_host_asyn_feed(buf, len, _this->m_serial_host);
    }
    sm_host_process(_this->m_serial_host);

    return 0;
}