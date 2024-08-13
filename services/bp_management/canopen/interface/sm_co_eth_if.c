//
// Created by vnbk on 16/03/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __linux
#include <unistd.h>
#endif

#include "sm_co_if.h"
#include "linux/utils/tcp_client.h"

#define PACKET_LENGTH                               13
#define FRAME_ID_LENGTH                             4
#define FRAME_DATA_LENGTH                           8

#define MAX_PACKET            50
#define MAX_BUFFER            (PACKET_LENGTH)

#define FRAME_INFO_DEFAULT        0x00
#define FRAME_INFO_LENGTH_MASK    0x0F

#define TAG "sm_co_ethernet_if"

typedef struct sm_co_if_packet {
    uint8_t m_info;
    uint32_t m_frame_id;
    uint8_t m_data_len;
    uint8_t m_frame_data[FRAME_DATA_LENGTH];
}sm_co_if_packet_t;

struct sm_co_if{
    tcp_client_t* m_tcp_client;
    int m_port;
    const char* m_host;

    sm_co_if_recv_callback_fn_t m_callback;
};

static sm_co_if_t g_sm_co_if_df = {
            .m_tcp_client = NULL,
            .m_port = 0,
            .m_host = NULL
        };

sm_co_if_t* sm_co_if_create(uint8_t _type, const char *host, int port, void* _arg){
    if(_type == CO_ETHERNET_CANBUS_IF) {
        tcp_client_t *client = tcp_client_create();
        if (!client) {
            return NULL;
        }
        sm_co_if_t *obj = malloc(sizeof(sm_co_if_t));
        if (!obj) {
            free(client);
            return NULL;
        }
        obj->m_tcp_client = client;
        obj->m_host = host;
        obj->m_port = port;
        return obj;
    }
    return NULL;
}

sm_co_if_t* sm_co_if_create_default(uint8_t _type, const char *host, int port, void* _arg){
    if(_type == CO_ETHERNET_CANBUS_IF) {
        tcp_client_t *client = tcp_client_create_default();
        if (!client) {
            return NULL;
        }
        g_sm_co_if_df.m_tcp_client = client;
        g_sm_co_if_df.m_host = host;
        g_sm_co_if_df.m_port = port;

        return &g_sm_co_if_df;
    }
    return NULL;
}

int sm_co_if_free(sm_co_if_t* self){
    if(!self){
        return -1;
    }
    if(tcp_client_is_connected(self->m_tcp_client)){
        tcp_client_disconnect(self->m_tcp_client);
        self->m_tcp_client = NULL;
    }
    return 0;
}

int sm_co_if_set_config(sm_co_if_t *self, const char *_argv, int _argc, void* _arg){
    if(!self){
        return -1;
    }
    self->m_host = _argv;
    self->m_port = _argc;

    return 0;
}


int sm_co_if_reg_recv_callback(sm_co_if_t* self, sm_co_if_recv_callback_fn_t callback_fn){
    if(!self)
        return -1;

    self->m_callback = callback_fn;
    return 0;
}

void* sm_co_if_get_interface(sm_co_if_t* _this){
    if(!_this){
        return NULL;
    }
    return _this->m_tcp_client;
}

int sm_co_if_connect(sm_co_if_t* self){
    if(self){
        return tcp_client_connect(self->m_tcp_client, self->m_host, self->m_port);
    }
    return -1;
}

int sm_co_if_disconnect(sm_co_if_t* self){
    if(self){
        return tcp_client_disconnect(self->m_tcp_client);
    }
    return -1;
}

int sm_co_if_is_connected(sm_co_if_t* self){
    if(self){
        return tcp_client_is_connected(self->m_tcp_client);
    }
    return -1;
}

int sm_co_if_send(sm_co_if_t* self, uint32_t frame_id, const unsigned char* data, int len, int timeout){
    if(!self || !tcp_client_is_connected(self->m_tcp_client)){
        return -1;
    }
    
    uint8_t buf[PACKET_LENGTH] = {0,};
    int index = 0;
    buf[index++] = FRAME_INFO_DEFAULT | (len & FRAME_INFO_LENGTH_MASK);
    buf[index++] = (frame_id >> 24) & 0xFF;
    buf[index++] = (frame_id >> 16) & 0xFF;
    buf[index++] = (frame_id >> 8) & 0xFF;
    buf[index++] = frame_id & 0xFF;
    for(int i = 0; i < len; i++){
        buf[index++] = data[i];
    }
    for(;index < PACKET_LENGTH; index++){
        buf[index] = 0;
    }

//    LOG_DBG(TAG, "Send CanOpen message with cob-id: 0x%2X", frame_id);

    return tcp_client_send(self->m_tcp_client, buf, index, timeout);
}

int sm_co_if_recv(sm_co_if_t* self, unsigned char* buf, int max_len, int timeout){
    if(self){
        return tcp_client_recv(self->m_tcp_client, buf, max_len, timeout);
    }
    return -1;
}

static int sm_co_if_decode_packet(const uint8_t* data, int len, sm_co_if_packet_t* packets){
    if(len < PACKET_LENGTH){
        return -1;
    }
    sm_co_if_packet_t* packet = packets;
    int number = 0;

    for(int i = 0; i < len; i += PACKET_LENGTH){
        memset(packet, 0, sizeof(sm_co_if_packet_t));

        packet->m_info = data[i];
        packet->m_data_len = packet->m_info & FRAME_INFO_LENGTH_MASK;

        packet->m_frame_id |= ((uint32_t)data[i+1] << 24);
        packet->m_frame_id |= ((uint32_t)data[i+2] << 16);
        packet->m_frame_id |= ((uint32_t)data[i+3] << 8);
        packet->m_frame_id |= ((uint32_t)data[i+4] << 0);

        memcpy(packet->m_frame_data, &data[i+5], FRAME_DATA_LENGTH);

        packet = &packets[number++];

//            printf("CanBus Packet Info, data length = %d\n", packet->m_info & FRAME_INFO_LENGTH_MASK);
            printf("CanBus Packet Frame ID: 0x%02X\n", packet->m_frame_id);
          /*  printf("CanBus Packet: ");
            for(int index = 0; index < 8; index++){
                printf("0x%2X- ", data[index+5]);
            }*/
    }
    return number;
}

int sm_co_if_process(sm_co_if_t* self){
    if(!self) {
        return -1;
    }
    if(!tcp_client_is_connected(self->m_tcp_client)){
        if(tcp_client_connect(self->m_tcp_client, self->m_host, self->m_port) < 0){
            usleep(1000*1000);
            return 0;
        }
    }
    uint8_t buf[20];
    int len = tcp_client_recv(self->m_tcp_client, buf, MAX_BUFFER, 1);
    if(len > 0){
//        printf("Data len: %d\n", len);
        sm_co_if_packet_t packet[MAX_PACKET];
        int packet_number = sm_co_if_decode_packet(buf, len, packet);
        if(packet_number <= 0){
            return 0;
        }
        if(self->m_callback){
            self->m_callback(packet->m_frame_id, packet->m_frame_data);
        }
    }
    return len;
}