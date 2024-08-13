//
// Created by vnbk on 16/03/2023.
//

#ifndef sm_co_if_H
#define sm_co_if_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define HOST_DEFAULT "192.168.1.254"
#define PORT_DEFAULT 32000

#define SERIAL_PORT_DEFAULT "/dev/ttyUSB0"

enum {
    CO_ETHERNET_CANBUS_IF,
    CO_SERIAL_CANBUS_IF,
    CO_MODBUS_CANBUS_IF
};

typedef struct sm_co_if sm_co_if_t;

typedef void (*sm_co_if_receive_callback_fn)(const uint32_t, uint8_t*);

sm_co_if_t *sm_co_if_create(uint8_t _type, const char *_argv, int _argc, void* _arg);

sm_co_if_t *sm_co_if_create_default(uint8_t _type, const char *_argv, int _argc, void* _arg);

int sm_co_if_free(sm_co_if_t* self);

int sm_co_if_reg_recv_callback(sm_co_if_t* self, sm_co_if_receive_callback_fn callback_fn);

int sm_co_if_set_config(sm_co_if_t *self, const char *_argv, int _argc, void* _arg);

int sm_co_if_connect(sm_co_if_t *self);

int sm_co_if_disconnect(sm_co_if_t *self);

int sm_co_if_is_connected(sm_co_if_t *self);

int sm_co_if_send(sm_co_if_t *self, uint32_t frame_id, const unsigned char *data, int len, int timeout);

int sm_co_if_recv(sm_co_if_t *self, unsigned char *buf, int max_len, int timeout);

int sm_co_if_process(sm_co_if_t *self);

#ifdef __cplusplus
}
#endif

#endif //sm_co_if_H
