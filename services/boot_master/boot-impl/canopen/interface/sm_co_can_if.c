#include <canopen/interface/sm_co_if.h>
#include "can_hw.h"
#include "CO_CAN_Msg.h"
#include "str_service.h"

struct sm_co_if{
    int32_t m_baud;
    sm_co_if_receive_callback_fn m_callback;
};

static sm_co_if_t g_canopen_if = {
        .m_callback = NULL,
};

static void sm_app_co_can_send_impl(CO_CAN_Msg *p_msg) {
	g_can_module.can_tx.u32Id = p_msg->id.can_id;
	g_can_module.can_tx.eIdType = eCANFD_SID;
	g_can_module.can_tx.eFrmType = eCANFD_DATA_FRM;
	g_can_module.can_tx.bBitRateSwitch = 0;
	g_can_module.can_tx.u32DLC = p_msg->data_len;
	my_memcpy(g_can_module.can_tx.au8Data, p_msg->data, p_msg->data_len);
	sm_board_can_hw_send(&g_can_module);
}

int32_t sm_canopen_send(const uint8_t* _data, int32_t _len){
	CO_CAN_Msg p_msg_;
	my_memcpy(p_msg_.data, _data, _len);
	p_msg_.data_len = _len;
	sm_app_co_can_send_impl(&p_msg_);
	return 1;
}
int sm_co_if_send(sm_co_if_t *self, uint32_t frame_id, const unsigned char *data, int len, int timeout){
    if(!self){
        return -1;
    }

	g_can_module.can_tx.u32Id = frame_id;
	g_can_module.can_tx.eIdType = eCANFD_SID;
	g_can_module.can_tx.eFrmType = eCANFD_DATA_FRM;
	g_can_module.can_tx.bBitRateSwitch = 0;
	g_can_module.can_tx.u32DLC = len;
	my_memcpy(g_can_module.can_tx.au8Data, data, len);
	sm_board_can_hw_send(&g_can_module);
}

sm_co_if_t *sm_co_if_create_default(uint8_t _type, const char *host, int port, void* _arg){
        g_canopen_if.m_callback = (sm_co_if_receive_callback_fn)_arg;
        return &g_canopen_if;

}

int sm_co_if_reg_recv_callback(sm_co_if_t* self, sm_co_if_receive_callback_fn callback_fn){
	self->m_callback = callback_fn;
	return 0;
}


