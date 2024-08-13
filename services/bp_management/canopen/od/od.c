/*
 * od.c
 *
 *  Created on: Oct 12, 2022
 *      Author: Dakaka
 */


#include "od.h"

/*--------------------------------------------------------------------------------------------
 * 1.2 Declare and define all manufacturer-specific profile sub-object
 */

/* USER CODE 1 BEGIN */

CO_Sub_Object_Ext_t g_ext_boot_state;
CO_Sub_Object_Ext_t g_ext_boot_request;

CO_Sub_Object boot_param[] = {
        {NULL, ODA_SDO_RW, 3, NULL},
        {NULL, ODA_SDO_RW, 4, NULL},
        {NULL, ODA_SDO_RW, 2, NULL},
        {NULL, ODA_SDO_RW, 4, NULL},
        {NULL, ODA_SDO_RW, SEGMENT_MEMORY_SIZE, NULL},
        {NULL, ODA_SDO_RW, 2, NULL},
        {NULL, ODA_SDO_RW, 1, &g_ext_boot_state},
        {NULL, ODA_SDO_RW, 1, &g_ext_boot_request},
        {NULL, ODA_SDO_RW, 1, NULL},

}; /* 0x2001 ARRAY Boot-loader parameter */

/* USER CODE 1 END */

/*******************************************************************************************************
 * 			Declare and define OD
 *
 * 			WARNING: Must sort the object so that the index value ascending from smallest to largest!
 *******************************************************************************************************/
static CO_Object co_od_list[]=
{
		/*** [0x1000- 0x1FFF] Communication profile objects***/

		/** General communication object**/
		{.index = 0x1000, .sub_number = 1, .subs = obj_1000_device_type},
		{.index = 0x1001, .sub_number = 1, .subs = obj_1001_error_reg},
		{.index = 0x1002, .sub_number = 1, .subs = obj_1002_status},
		{.index = 0x1003, .sub_number = 9, .subs = obj_1003_error_field},

		{.index = 0x1005, .sub_number = 1, .subs = obj_1005_sync_cob_id},
		{.index = 0x1006, .sub_number = 1, .subs = obj_1006_comm_cycle},
		{.index = 0x1007, .sub_number = 1, .subs = obj_1007_sync_window_len},
		{.index = 0x1008, .sub_number = 1, .subs = obj_1008_device_name},
		{.index = 0x1009, .sub_number = 1, .subs = obj_1009_hw_version},
		{.index = 0x100A, .sub_number = 1, .subs = obj_100A_sw_version},
		{.index = 0x100C, .sub_number = 1, .subs = obj_100C_guard_time},
		{.index = 0x100D, .sub_number = 1, .subs = obj_100D_life_time_factor},
		{.index = 0x1010, .sub_number = 3, .subs = obj_1010_store_para},
		{.index = 0x1011, .sub_number = 3, .subs = obj_1011_restore_default_para},
		{.index = 0x1012, .sub_number = 1, .subs = obj_1012_time_cob_id},
		{.index = 0x1013, .sub_number = 1, .subs = obj_1013_high_res_timestamp},
		{.index = 0x1014, .sub_number = 1, .subs = obj_1014_emcy_cob_id},
		{.index = 0x1015, .sub_number = 1, .subs = obj_1015_inhibit_time_emcy},
		{.index = 0x1016, .sub_number = 6, .subs = obj_1016_consumer_heartbeat},
		{.index = 0x1017, .sub_number = 1, .subs = obj_1017_producer_heartbeat},
		{.index = 0x1018, .sub_number = 4, .subs = obj_1018_identity},
		{.index = 0x1019, .sub_number = 1, .subs = obj_1019_sync_cter_over_flow_value},
		{.index = 0x1030, .sub_number = 1, .subs = obj_1030_sync_bit_pos_over_flow_value},
		{.index = 0x1031, .sub_number = 3, .subs = obj_1031_sdo_server_comm_para},
		{.index = 0x1032, .sub_number = 2, .subs = obj_1032_sdo_client_comm_para},
		/** SDO parameter object [0x1200- 0x12FF] **/
		/** PDO parameter object [0x1400- 0x1BFF] **/

		/*** [0x2000- 0x5FFF] Manufacturer-specific profile object ***/

		/* USER CODE 2 begin [0x2000- 0x4FFF] -----------------------------------------------------------*/

        {.index = SDO_BOOTLOADER_INDEX, .sub_number = 9, .subs = boot_param}	/* 0x2001 ARRAY Bootloader parameter */
		/* USER CODE 2 the end ---------------------------------------------------------*/
};

static CO_OD co_od =
{
		(sizeof(co_od_list) / sizeof(co_od_list[0])), co_od_list
};

CO_OD *p_co_od = &co_od;
