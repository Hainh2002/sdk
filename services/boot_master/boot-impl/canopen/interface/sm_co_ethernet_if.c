//
// Created by vnbk on 16/03/2023.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "can_hw.h"
#include <CO.h>
#include "CO_CAN_Msg.h"
#ifdef __linux
#include <unistd.h>
#endif

#include <canopen/interface/sm_co_if.h>

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
	CAN_Hw*	m_can;
    int m_port;
    const char* m_host;

    sm_co_if_receive_callback_fn m_callback;
};

static sm_co_if_t g_sm_co_if_df = {
            .m_port = 0,
            .m_host = NULL
        };

sm_co_if_t* sm_co_if_create(uint8_t _type, const char *host, int port, void* _arg){

    return NULL;
}

sm_co_if_t* sm_co_if_create_default(uint8_t _type, const char *host, int port, void* _arg){

    return NULL;
}

int sm_co_if_free(sm_co_if_t* self){
    if(!self){
        return -1;

    return 0;
}

int sm_co_if_set_config(sm_co_if_t *self, const char *_argv, int _argc, void* _arg){
    if(!self){
        return -1;
    }

    return 0;
}


int sm_co_if_reg_recv_callback(sm_co_if_t* self, sm_co_if_receive_callback_fn callback_fn){
    if(!self)
        return -1;

    self->m_callback = callback_fn;
    return 0;
}

int sm_co_if_connect(sm_co_if_t* self){
    if(self){
    }
    return -1;
}

int sm_co_if_disconnect(sm_co_if_t* self){
    if(self){

    }
    return -1;
}

int sm_co_if_is_connected(sm_co_if_t* self){
    if(self){
    }
    return -1;
}

static void sm_app_co_can_send_impl(CO_CAN_Msg *p_msg) {
	g_can_module.can_tx.u32Id = p_msg->id.can_id;
	g_can_module.can_tx.eIdType = eCANFD_SID;
	g_can_module.can_tx.eFrmType = eCANFD_DATA_FRM;
	g_can_module.can_tx.bBitRateSwitch = 0;
	g_can_module.can_tx.u32DLC = p_msg->data_len;
	my_memcpy(g_can_module.can_tx.au8Data, p_msg->data, p_msg->data_len);
	sm_board_can_hw_send(&g_can_module);
}

int sm_co_if_send(sm_co_if_t* self, uint32_t frame_id, const unsigned char* data, int len, int timeout){


//    printf("Send CanOpen message with cob-id: 0x%2X\n", frame_id);
	mgs.id.can_id = _cod_id;
	mgs.data_len = 0;
	sm_app_co_can_send_impl(&mgs);
	CANFD_TransmitTxMsg(self->m_can->can_handle, 0, data);
	sm_app_co_can_send_impl()

    return 1;
}

int sm_co_if_recv(sm_co_if_t* self, unsigned char* buf, int max_len, int timeout){
    if(self){
//        return tcp_client_recv(self->m_tcp_client, buf, max_len, timeout);
    }
    return -1;
}


int sm_co_if_process(sm_co_if_t* self){
    if(!self) {
        return -1;
    }
//    if(!tcp_client_is_connected(self->m_tcp_client)){
//        if(tcp_client_connect(self->m_tcp_client, self->m_host, self->m_port) < 0){
//            usleep(1000*1000);
//            return 0;
//        }
//        printf("Connected SUCCESS\n");
//    }
//    uint8_t buf[20];
//    int len = tcp_client_recv(self->m_tcp_client, buf, MAX_BUFFER, 1);
//    if(len > 0){
////        printf("Data len: %d\n", len);
//        sm_co_if_packet_t packet[MAX_PACKET];
//        int packet_number = sm_co_if_decode_packet(buf, len, packet);
//        if(packet_number <= 0){
//            return 0;
//        }
//        if(self->m_callback){
//            self->m_callback(packet->m_frame_id, packet->m_frame_data);
//        }
//    }
//    return len;
}
