/*
 * app_co_init.c
 *
 *  Created on: Jun 17, 2022
 *      Author: Dakaka
 */

#include <canopen/interface/sm_co_if.h>
#include <canopen/interface/sm_co_if.h>
#include "app_co_init.h"
#include "od.h".
#include "CO.h"
#include "can_hw.h"

#include "sm_elapsed_timer.h"

CO CO_DEVICE;

static elapsed_timer_t g_co_timer;
static sm_co_if_t* g_co_if = NULL;

static app_co_received_callback_fn_t g_callback;
static void* g_arg;

/********************************************************************************************
 * 									MANDATORY: Edit carefully !!!
 * *******************************************************************************************/
/* Declare for build TPDO message function when using "tpdo_build_data_manually" mode*/
static void tpdo1_build_data_impl(uint8_t* buffer);
static void tpdo2_build_data_impl(uint8_t* buffer);
static void tpdo3_build_data_impl(uint8_t* buffer);
static void tpdo4_build_data_impl(uint8_t* buffer);

void* tpdo_build_data_impl[TPDO_NUMBER] =
{
		tpdo1_build_data_impl,
		tpdo2_build_data_impl,
		tpdo3_build_data_impl,
		tpdo4_build_data_impl
};

static void tpdo1_build_data_impl(uint8_t* buffer)
{
	(void)buffer;
//    printf("TPDO_1 Handler\n");
//    printf("Data: ");
//    for(int index = 0; index < 8; index++){
//        printf("0x%02x-", buffer[index]);
//    }
//    printf("\n");
}

static void tpdo2_build_data_impl(uint8_t* buffer)
{
	(void)buffer;
//    printf("TPDO_2 Handler\n");
//    printf("Data: ");
//    for(int index = 0; index < 8; index++){
//        printf("0x%02x-", buffer[index]);
//    }
//    printf("\n");
}

static void tpdo3_build_data_impl(uint8_t* buffer)
{
	(void)buffer;
//    printf("TPDO_3 Handler\n");
//    printf("Data: ");
//    for(int index = 0; index < 8; index++){
//        printf("0x%02x-", buffer[index]);
//    }
//    printf("\n");
}

static void tpdo4_build_data_impl(uint8_t* buffer)
{
	(void)buffer;
//    printf("TPDO_4 Handler\n");
//    printf("Data: ");
//    for(int index = 0; index < 8; index++){
//        printf("0x%02x-", buffer[index]);
//    }
//    printf("\n");
}

/* Define can_send message function */
static void app_co_can_send_impl(CO_CAN_Msg* p_msg)
{
    if(g_co_if){
        sm_co_if_send(g_co_if, p_msg->id.cob_id, p_msg->data, p_msg->data_len, 100);
    }
}

/* Call in receive can interrupt */
void app_co_can_receive(const uint32_t can_id, uint8_t* data)
{
	if(CO_can_receive_basic_handle(&CO_DEVICE, can_id, data)) return;
	/* USER CODE BEGIN */

    if(g_callback){
        g_callback(can_id, data, g_arg);
    }

	/* USER CODE END */
}

/* Define erase_and_write_data_to_flash function
 * Return: success(0), fail(-1)
 */
int8_t app_co_erase_and_write_data_to_flash_impl(uint32_t addr, uint32_t buff, uint32_t len)
{

}

/* Define flash_error_handle function */
void app_co_flash_error_handle_impl(void)
{

}

/* Define set CAN baud-rate function */
void app_co_set_can_baudrate_impl(uint32_t br_khz)
{

}

sm_co_if_t* app_co_get_if(){
    return g_co_if;
}

void app_co_set_if(sm_co_if_t* _co_if){
    if(_co_if){
        g_co_if = _co_if;
        sm_co_if_reg_recv_callback(g_co_if, app_co_can_receive);
    }
}

void app_co_sync(uint8_t _enable){
    if(_enable){
        CO_disable_sync_mode(&CO_DEVICE);
    }else{
        CO_recover_sync_mode(&CO_DEVICE, &od_temp_comm_profile_para);
    }
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

void app_co_init(sm_co_if_t* _co_if, app_co_received_callback_fn_t _fn_callback, void* _arg)
{
    g_co_if = _co_if;
    if(!g_co_if){
        return;
    }
    sm_co_if_reg_recv_callback(g_co_if, app_co_can_receive);

    /* [Mandatory] Set CO_CAN_send interface */
    CO_CAN_set_can_send_interface(sm_app_co_can_send_impl);

	/* [Mandatory] Init CO object */
#if CO_FLASH__USE
	CO_FLASH_init(&CO_DEVICE.flash,
			app_co_erase_and_write_data_to_flash_impl,
			app_co_flash_error_handle_impl);
#endif
#if APP_OD_STORAGE__USE
#if CO_FLASH__USE & (SW_VERSION != 0)
	CO_OD_STORAGE_init(&CO_DEVICE.storage,
			APP_OD_STORAGE__STORAGE_REGION__ADDR,
			APP_OD_STORAGE__STORAGE_REGION__SIZE,
			&od_temp_comm_profile_para,
			&od_temp_manu_profile_para,
			sizeof(OD_Temp_Manufacturer_Profile_Para_t),
			&CO_DEVICE.flash,
			p_co_od,
			SW_VERSION,
			IS_MAIN_APP);
#else
//	while(1); /* Code fail */
#endif
#endif

	CO_init_basic(&CO_DEVICE,
			od_temp_comm_profile_para.x1000_device_type,
			&od_temp_comm_profile_para.x1018_identity,
			p_co_od);

	CO_SYNC_init(&CO_DEVICE.sync, &od_temp_comm_profile_para);

	for(uint8_t i = 0; i < TPDO_NUMBER; i++)
	{
		CO_TPDO_init(&CO_DEVICE.tpdos[i],
				&CO_DEVICE.sync,
				&od_temp_comm_profile_para.x1A0x_tpdo_map_para[i],
				CO_DEVICE.p_od,
				&od_temp_comm_profile_para.x180x_tpdo_comm_para[i],
				tpdo_build_data_impl[i]);
	}
	CO_SDOserver_init(&CO_DEVICE.sdo_server,
			&od_temp_comm_profile_para,
			&CO_DEVICE.sync,
			CO_DEVICE.p_od);
	CO_SDOclient_init(&CO_DEVICE.sdo_client,
			&od_temp_comm_profile_para,
			&CO_DEVICE.sync);

	CO_set_node_id(&CO_DEVICE, od_temp_manu_profile_para.x500300_node_ID);
	app_co_set_can_baudrate_impl(od_temp_manu_profile_para.x500301_baud_rate_kHz);

    CO_recover_sync_mode(&CO_DEVICE, &od_temp_comm_profile_para);

	/* USER CODE BEGIN */

    elapsed_timer_resetz(&g_co_timer, 1);

    g_callback = _fn_callback;
    g_arg = _arg;
	/* USER CODE END */
}


/********************************************************************************************
 * 									USER CODE
 * *******************************************************************************************/

void app_co_free(void){
    if(g_co_if){
        sm_co_if_free(g_co_if);
        g_co_if = NULL;
    }
}

void app_co_process(void){
    if(g_co_if) {
//        sm_co_if_process(g_co_if);
        if(!elapsed_timer_get_remain(&g_co_timer)){
            CO_process(&CO_DEVICE, 1);
            elapsed_timer_reset(&g_co_timer);
        }
    }
}
