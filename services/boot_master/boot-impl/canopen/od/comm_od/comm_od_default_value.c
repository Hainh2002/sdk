/*
 * app_co_comm_od_default_value.c
 *
 *  Created on: Oct 7, 2022
 *      Author: Dakaka
 */

#include "comm_od_default_value.h"
#include "od_config.h"

/************************************************************************************
 *					User set default value for CANOPEN communication
 ************************************************************************************/

#define DEFAULT__1000__DEVICE_TYPE						0x00000000			/*Type: 32bit*/

/* Synchronous schedule */
#define DEFAULT__1005__SYNC_ENABLE						CO_SYNC_VALID		/*	Enable sync mode: Set CO_SYNC_VALID (0)
																				Disable sync mode: set CO_SYNC_NOT_VALID (1) */
#define DEFAULT__1005__SYNC_GEN							CO_SYNC_GEN		    /*	Producer sync: Set CO_SYNC_GEN            -> For PMU/BSS_MASTER
																				Consumer sync: Set CO_SYNC_NOT_GEN        -> For MC/BP/... */
#define DEFAULT__1006__COMM_CYCLE_PERIOD_MS				(10U)				/* Set according to synchronous scheduled */
#define DEFAULT__1007__SYNC_WINDOW_LENGTH_MS			(5U)				/* Set according to synchronous scheduled */
#define DEFAULT__1019__SYNC_CTER_OVER_FLOW_VALUE		(150U)				/* Set according to synchronous scheduled. Setting range: 1-30000 */
#define DEFAULT__1030__SYNC_BIT_POS_OVER_FLOW_VALUE		(1U)				/* Set according to synchronous scheduled. Setting range: 0-31 */

/*  */
#define DEFAULT__1008__DEVICE_NAME						{'S','E','L','E','X','_','B','P','M'}	/*(12 byte)*/

#define DEFAULT__1009__HW_VERSION						0x00000000			/*(32 bit)*/
#define DEFAULT__100A__SW_VERSION						SW_VERSION			/*(32 bit)*/

#define DEFAULT__100C__GUARD_TIME_MS					0x0000    			/*(16 bit)*/
#define DEFAULT__100D__LIFE_TIME_FACTOR					0x00      			/*(8 bit)*/

/* Emergency */
#define DEFAULT__1014__ENABLE_EMCY						CO_EMCY_NOT_VALID	/*	Enable >> Set CO_EMCY_VALID			(0)
																				Disable >> Set CO_EMCY_NOT_VALID	(1) */
#define DEFAULT__1015__INHIBIT_TIME_EMCY				0x0000				/*(16 bit)*/

/* Consumer heart-beat time */
#define DEFAULT__1016__CONSUMER_HEARTBEAT_TIME_SUB0		0x00				/*(8 bit)*/
#define DEFAULT__1016__CONSUMER_HEARTBEAT_TIME0			0x00000000 			/*(32 bit), mili-seconds*/
#define DEFAULT__1016__CONSUMER_HEARTBEAT_TIME1			0x00000000 			/*(32 bit), mili-seconds*/
#define DEFAULT__1016__CONSUMER_HEARTBEAT_TIME2			0x00000000 			/*(32 bit), mili-seconds*/
#define DEFAULT__1016__CONSUMER_HEARTBEAT_TIME3			0x00000000 			/*(32 bit), mili-seconds*/
#define DEFAULT__1016__CONSUMER_HEARTBEAT_TIME4			0x00000000 			/*(32 bit), mili-seconds*/
/* Producer heart-beat time */
#define DEFAULT__1017__PRODUCER_HEARTBEAT_TIME			0x00000000			/*(32 bit), mili-seconds*/

/* Identity */
#define DEFAULT__101800__VENDER_ID						0x00000000			/*(32 bit)*/
#define	DEFAULT__101801__PRODUCT_CODE					0x00000000			/*(32 bit)*/
#define	DEFAULT__101802__REVISION_NUMBER				0x00000000			/*(32 bit)*/
#define	DEFAULT__101803__SERIAL_NUMBER					0x00000000			/*(32 bit)*/

/* SDO Server */
#define DEFAULT__103101__SDO_SERV_TIMEOUT_MS				(1000)				/*	(16 bit). Suggest default = 1000*/

/* SDO client */
#define DEFAULT__103200__SDO_CLI_SYNC_TRANS_TYPE			(0)					//<< Set according to synchronous scheduled
																			//   Set 0 if you want disable sync mode, switch to always communication mode
#define DEFAULT__103201__SDO_CLI_SYNC_MASK_REGISTER		(0x00000000)		//<< Set according to synchronous scheduled, default: 0x00000000

/* RPDO1 Communication Parameter => Fill in .x140x_rpdo_comm_para[0] */
/* RPDO2 Communication Parameter => Fill in .x140x_rpdo_comm_para[1] */
/* RPDO3 Communication Parameter => Fill in .x140x_rpdo_comm_para[2] */
/* RPDO4 Communication Parameter => Fill in .x140x_rpdo_comm_para[3] */

/* RPDO1 Mapping Parameter => Fill in .x160x_rpdo_map_para[0]*/
/* RPDO2 Mapping Parameter => Fill in .x160x_rpdo_map_para[1]*/
/* RPDO3 Mapping Parameter => Fill in .x160x_rpdo_map_para[2]*/
/* RPDO4 Mapping Parameter => Fill in .x160x_rpdo_map_para[3]*/

/* TPDO1 Communication Parameter*/
#define DEFAULT__180000__TPDO1_ENABLE						CO_PDO_NOT_VALID	//<< Set CO_PDO_VALID if used, otherwise, set CO_PDO_NOT_VALID
#define DEFAULT__180001__TPDO1_TRANS_TYPE					(00)				//<< Set according to synchronous scheduled.
																				//	 Detail: See CO_TPDO.h >> TPDO transmission Types
#define DEFAULT__180002__TPDO1_MAX_INHIBIT_TIME_MS			(1000)				/*	(16 bit). Suggest default = 1000. Valid when setting TPDO in event mode*/
#define DEFAULT__180003__TPDO1_EVENT_TIME_MS				(10000)				/*	(16 bit). Suggest default = 10000. Valid when setting TPDO in event mode*/
#define DEFAULT__180005__TPDO1_SYNC_MASK_REGISTER			(0x00000000)		//<< Set according to synchronous scheduled, default: 0x00000000
#define DEFAULT__180006__TPDO1_BUILD_DATA_OPTION			(CO_TPDO_build_data_manually)					//<< Default: CO_TPDO_build_data_manually
																				//see TPDO.h/CO_TPDO_Build_Data_option_t
/* TPDO2 Communication Parameter*/
#define DEFAULT__180100__TPDO2_ENABLE						CO_PDO_NOT_VALID	//<< Set CO_PDO_VALID if used, otherwise, set CO_PDO_NOT_VALID
#define DEFAULT__180101__TPDO2_TRANS_TYPE					(00)				//<< Set according to synchronous scheduled.
																				//	 Detail: See CO__TPDO.h >> TPDO transmission Types
#define DEFAULT__180102__TPDO2_MAX_INHIBIT_TIME_MS			(1000)				/*	(16 bit). Suggest default = 1000. Valid when setting TPDO in event mode*/
#define DEFAULT__180103__TPDO2_EVENT_TIME_MS				(10000)				/*	(16 bit). Suggest default = 10000. Valid when setting TPDO in event mode*/
#define DEFAULT__180105__TPDO2_SYNC_MASK_REGISTER			(0x00000000)		//<< Set according to synchronous scheduled, default: 0x00000000
#define DEFAULT__180106__TPDO2_BUILD_DATA_OPTION			(CO_TPDO_build_data_manually)					//<< Default: CO_TPDO_build_data_manually
																				//see TPDO.h/CO_TPDO_Build_Data_option_t
/* TPDO3 Communication Parameter*/
#define DEFAULT__180200__TPDO3_ENABLE						CO_PDO_NOT_VALID	//<< Set CO_PDO_VALID if used, otherwise, set CO_PDO_NOT_VALID
#define DEFAULT__180201__TPDO3_TRANS_TYPE					(00)				//<< Set according to synchronous scheduled.
																				//	 Detail: See CO__TPDO.h >> TPDO transmission Types
#define DEFAULT__180202__TPDO3_MAX_INHIBIT_TIME_MS			(1000)				/*	(16 bit). Suggest default = 1000. Valid when setting TPDO in event mode*/
#define DEFAULT__180203__TPDO3_EVENT_TIME_MS				(10000)				/*	(16 bit). Suggest default = 10000. Valid when setting TPDO in event mode*/
#define DEFAULT__180205__TPDO3_SYNC_MASK_REGISTER			(0x00000000)		//<< Set according to synchronous scheduled, default: 0x00000000
#define DEFAULT__180206__TPDO3_BUILD_DATA_OPTION			(CO_TPDO_build_data_manually)					//<< Default: CO_TPDO_build_data_manually
																				//see TPDO.h/CO_TPDO_Build_Data_option_t

/* TPDO4 Communication Parameter*/
#define DEFAULT__180300__TPDO4_ENABLE						CO_PDO_NOT_VALID	//<< Set CO_PDO_VALID if used, otherwise, set CO_PDO_NOT_VALID
#define DEFAULT__180301__TPDO4_TRANS_TYPE					(00)				//<< Set according to synchronous scheduled.
																				//	 Detail: See CO__TPDO.h >> TPDO transmission Types
#define DEFAULT__180302__TPDO4_MAX_INHIBIT_TIME_MS			(1000)				/*	(16 bit). Suggest default = 1000. Valid when setting TPDO in event mode*/
#define DEFAULT__180303__TPDO4_EVENT_TIME_MS				(10000)				/*	(16 bit). Suggest default = 10000. Valid when setting TPDO in event mode*/
#define DEFAULT__180305__TPDO4_SYNC_MASK_REGISTER			(0x00000000)		//<< Set according to synchronous scheduled, default: 0x00000000
#define DEFAULT__180306__TPDO4_BUILD_DATA_OPTION			(CO_TPDO_build_data_manually)					//<< Default: CO_TPDO_build_data_manually
																				//see TPDO.h/CO_TPDO_Build_Data_option_t

/* TPDO1 Mapping Parameter => Fill in .x1A0x_tpdo_map_para[0] */
/* TPDO2 Mapping Parameter => Fill in .x1A0x_tpdo_map_para[1] */
/* TPDO3 Mapping Parameter => Fill in .x1A0x_tpdo_map_para[2] */
/* TPDO4 Mapping Parameter => Fill in .x1A0x_tpdo_map_para[3] */


/*******************************************************************************************
 * Define and assign default value to some communication profile initializer data variable
 * 				WARNING:
 *				- Edit carefully !!!
 * *******************************************************************************************/


OD_Temp_Comm_Profile_Para_t od_temp_comm_profile_para =
{
	.x1000_device_type = DEFAULT__1000__DEVICE_TYPE,
	.x1005_cob_id_sync_message =
	{
			.node_id = 0,
			.func_code = CO_CAN_ID_SYNC >> 7,
			.reverse1 = 0,
			.bit_frame = CO_BIT_CAN_FRAME_11BIT,
			.bit_30 = DEFAULT__1005__SYNC_GEN,
			.valid = DEFAULT__1005__SYNC_ENABLE
	},
	.x1006_comm_cycle_period_ms = DEFAULT__1006__COMM_CYCLE_PERIOD_MS,
	.x1007_sync_window_length_ms = DEFAULT__1007__SYNC_WINDOW_LENGTH_MS,
	.x1008_device_name = DEFAULT__1008__DEVICE_NAME,
	.x1009_hardware_version = DEFAULT__1009__HW_VERSION,
	.x100A_software_version = DEFAULT__100A__SW_VERSION,
	.x100C_guard_time_ms = DEFAULT__100C__GUARD_TIME_MS,
	.x100D_life_time_factor = DEFAULT__100D__LIFE_TIME_FACTOR,
	.x1012_cob_id_time_stamp_object = 0x00000000,	//<< Unsupported function
	.x1014_cob_id_emcy =
	{
			.node_id = NODE_ID_DEFAULT,
			.func_code = CO_CAN_ID_EMERGENCY >> 7,
			.reverse1 = 0,
			.bit_frame = CO_BIT_CAN_FRAME_11BIT,
			.bit_30 = 0,
			.valid = DEFAULT__1014__ENABLE_EMCY
	},
	.x1015_inhibit_time_emcy = DEFAULT__1015__INHIBIT_TIME_EMCY,
    .x1016_consumer_heartbeat_time_sub0 = DEFAULT__1016__CONSUMER_HEARTBEAT_TIME_SUB0,
    .x1016_consumer_heartbeat_time =
		{ DEFAULT__1016__CONSUMER_HEARTBEAT_TIME0, DEFAULT__1016__CONSUMER_HEARTBEAT_TIME1,
				DEFAULT__1016__CONSUMER_HEARTBEAT_TIME2, DEFAULT__1016__CONSUMER_HEARTBEAT_TIME3,
				DEFAULT__1016__CONSUMER_HEARTBEAT_TIME4 },
    .x1017_producer_heartbeat_time = DEFAULT__1017__PRODUCER_HEARTBEAT_TIME,
    .x1018_identity = {
        .vendor_id = DEFAULT__101800__VENDER_ID,
        .product_code = DEFAULT__101801__PRODUCT_CODE,
        .revision_number = DEFAULT__101802__REVISION_NUMBER,
        .serial_number = DEFAULT__101803__SERIAL_NUMBER
    },
    .x1019_sync_cter_over_flow_value = DEFAULT__1019__SYNC_CTER_OVER_FLOW_VALUE,
	.x1030_sync_bit_pos_over_flow_value = DEFAULT__1030__SYNC_BIT_POS_OVER_FLOW_VALUE,

	.x1031_sdo_server_comm_para = {
		.node_id = NODE_ID_DEFAULT,
		.trans_type = CO_SDO_SERVER_TRANSM_TYPE_SYNC,
		.allow_timeout_ms = DEFAULT__103101__SDO_SERV_TIMEOUT_MS,
	},
	.x1032_sdo_client_comm_para = {
		.sync_mask_reg = DEFAULT__103201__SDO_CLI_SYNC_MASK_REGISTER,
		.trans_type = DEFAULT__103200__SDO_CLI_SYNC_TRANS_TYPE,
	},

//    .x1280_SDOClientParameter = {
//        .COB_IDClientToServerTx.cob_id = 0x80000000,	//not supported yet!
//        .COB_IDServerToClientRx.cob_id = 0x80000000,	//not supported yet!
//        .node_IDOfTheSDOServer = 0x01					//not supported yet!
//    },
    /* RPDO1 Communication Parameter*/
	.x140x_rpdo_comm_para[0] = {
		.id = {
				.node_id = 0,
				.func_code = CO_CAN_ID_RPDO_1 >>7,
				.reverse1 = 0,
				.bit_frame = CO_BIT_CAN_FRAME_11BIT,
				.bit_30 = 0,
				.valid = CO_PDO_NOT_VALID
		},
		.transmission_type = 0,
		.max_inhibit_time_ms = 0,
		.compatibility_entry = 0,
		.max_event_time_ms = 0,
		.sync_start_value = 0
	},
    /* RPDO2 Communication Parameter*/
	.x140x_rpdo_comm_para[1] = {
		.id = {
				.node_id = 0,
				.func_code = CO_CAN_ID_RPDO_2 >>7,
				.reverse1 = 0,
				.bit_frame = CO_BIT_CAN_FRAME_11BIT,
				.bit_30 = 0,
				.valid = CO_PDO_NOT_VALID
		},
		.transmission_type = 0,
		.max_inhibit_time_ms = 0,
		.compatibility_entry = 0,
		.max_event_time_ms = 0,
		.sync_start_value = 0
	},
    /* RPDO3 Communication Parameter*/
	.x140x_rpdo_comm_para[2] = {
		.id = {
				.node_id = 0,
				.func_code = CO_CAN_ID_RPDO_3 >>7,
				.reverse1 = 0,
				.bit_frame = CO_BIT_CAN_FRAME_11BIT,
				.bit_30 = 0,
				.valid = CO_PDO_NOT_VALID
		},
		.transmission_type = 0,
		.max_inhibit_time_ms = 0,
		.compatibility_entry = 0,
		.max_event_time_ms = 0,
		.sync_start_value = 0
	},
    /* RPDO4 Communication Parameter*/
	.x140x_rpdo_comm_para[3] = {
		.id = {
				.node_id = 0,
				.func_code = CO_CAN_ID_RPDO_4 >>7,
				.reverse1 = 0,
				.bit_frame = CO_BIT_CAN_FRAME_11BIT,
				.bit_30 = 0,
				.valid = CO_PDO_NOT_VALID
		},
		.transmission_type = 0,
		.max_inhibit_time_ms = 0,
		.compatibility_entry = 0,
		.max_event_time_ms = 0,
		.sync_start_value = 0
	},
    /* RPDO1 Mapping Parameter*/
	.x160x_rpdo_map_para[0] = {
		.object_number = 0x00,
		.app_object[0].w	= 0x00000000,
		.app_object[1].w	= 0x00000000,
		.app_object[2].w	= 0x00000000,
		.app_object[3].w	= 0x00000000,
		.app_object[4].w	= 0x00000000,
		.app_object[5].w	= 0x00000000,
		.app_object[6].w	= 0x00000000,
		.app_object[7].w	= 0x00000000
	},
    /* RPDO2 Mapping Parameter*/
	.x160x_rpdo_map_para[1] = {
		.object_number = 0x00,
		.app_object[0].w	= 0x00000000,
		.app_object[1].w	= 0x00000000,
		.app_object[2].w	= 0x00000000,
		.app_object[3].w	= 0x00000000,
		.app_object[4].w	= 0x00000000,
		.app_object[5].w	= 0x00000000,
		.app_object[6].w	= 0x00000000,
		.app_object[7].w	= 0x00000000
	},
    /* RPDO3 Mapping Parameter*/
	.x160x_rpdo_map_para[2] = {
		.object_number = 0x00,
		.app_object[0].w	= 0x00000000,
		.app_object[1].w	= 0x00000000,
		.app_object[2].w	= 0x00000000,
		.app_object[3].w	= 0x00000000,
		.app_object[4].w	= 0x00000000,
		.app_object[5].w	= 0x00000000,
		.app_object[6].w	= 0x00000000,
		.app_object[7].w	= 0x00000000
	},
    /* RPDO4 Mapping Parameter*/
	.x160x_rpdo_map_para[3] = {
		.object_number = 0x00,
		.app_object[0].w	= 0x00000000,
		.app_object[1].w	= 0x00000000,
		.app_object[2].w	= 0x00000000,
		.app_object[3].w	= 0x00000000,
		.app_object[4].w	= 0x00000000,
		.app_object[5].w	= 0x00000000,
		.app_object[6].w	= 0x00000000,
		.app_object[7].w	= 0x00000000
	},
    /* TPDO1 Communication Parameter*/
	.x180x_tpdo_comm_para[0] = {
		.id = {
			.node_id = NODE_ID_DEFAULT ,
			.func_code =  CO_CAN_ID_TPDO_1 >>7,
			.reverse1 = 0,
			.bit_frame = CO_BIT_CAN_FRAME_11BIT,
			.bit_30 = CO_BIT_RTR_NOT_ALLOW,
			.valid = DEFAULT__180000__TPDO1_ENABLE
		},
		.transmission_type = DEFAULT__180001__TPDO1_TRANS_TYPE,
		.max_inhibit_time_ms = DEFAULT__180002__TPDO1_MAX_INHIBIT_TIME_MS,
		.max_event_time_ms = DEFAULT__180003__TPDO1_EVENT_TIME_MS,
		.sync_start_value = 0,					//<< ignored
		.sync_mask_reg = DEFAULT__180005__TPDO1_SYNC_MASK_REGISTER,
		.build_data_option = DEFAULT__180006__TPDO1_BUILD_DATA_OPTION,
	},
    /* TPDO2 Communication Parameter*/
	.x180x_tpdo_comm_para[1] = {				//<< EDITABLE:
		.id = {
			.node_id = NODE_ID_DEFAULT ,
			.func_code =  CO_CAN_ID_TPDO_2 >>7,
			.reverse1 = 0,
			.bit_frame = CO_BIT_CAN_FRAME_11BIT,
			.bit_30 = CO_BIT_RTR_NOT_ALLOW,
			.valid = DEFAULT__180100__TPDO2_ENABLE
		},
		.transmission_type = DEFAULT__180101__TPDO2_TRANS_TYPE,
		.max_inhibit_time_ms = DEFAULT__180102__TPDO2_MAX_INHIBIT_TIME_MS,
		.max_event_time_ms = DEFAULT__180103__TPDO2_EVENT_TIME_MS,
		.sync_start_value = 0,					//<< ignored
		.sync_mask_reg = DEFAULT__180105__TPDO2_SYNC_MASK_REGISTER,
		.build_data_option = DEFAULT__180106__TPDO2_BUILD_DATA_OPTION,
	},
    /* TPDO3 Communication Parameter*/
	.x180x_tpdo_comm_para[2] = {				//<< EDITABLE:
		.id = {
			.node_id = NODE_ID_DEFAULT ,
			.func_code =  CO_CAN_ID_TPDO_3 >>7,
			.reverse1 = 0,
			.bit_frame = CO_BIT_CAN_FRAME_11BIT,
			.bit_30 = CO_BIT_RTR_NOT_ALLOW,
			.valid = DEFAULT__180200__TPDO3_ENABLE
		},
		.transmission_type = DEFAULT__180201__TPDO3_TRANS_TYPE,
		.max_inhibit_time_ms = DEFAULT__180202__TPDO3_MAX_INHIBIT_TIME_MS,
		.max_event_time_ms = DEFAULT__180203__TPDO3_EVENT_TIME_MS,
		.sync_start_value = 0,					//<< ignored
		.sync_mask_reg = DEFAULT__180205__TPDO3_SYNC_MASK_REGISTER,
		.build_data_option = DEFAULT__180206__TPDO3_BUILD_DATA_OPTION,
	},
    /* TPDO4 Communication Parameter*/
	.x180x_tpdo_comm_para[3] = {				//<< EDITABLE:
		.id = {
			.node_id = NODE_ID_DEFAULT ,
			.func_code =  CO_CAN_ID_TPDO_4 >>7,
			.reverse1 = 0,
			.bit_frame = CO_BIT_CAN_FRAME_11BIT,
			.bit_30 = CO_BIT_RTR_NOT_ALLOW,
			.valid = DEFAULT__180300__TPDO4_ENABLE
		},
		.transmission_type = DEFAULT__180301__TPDO4_TRANS_TYPE,
		.max_inhibit_time_ms = DEFAULT__180302__TPDO4_MAX_INHIBIT_TIME_MS,
		.max_event_time_ms = DEFAULT__180303__TPDO4_EVENT_TIME_MS,
		.sync_start_value = 0,					//<< ignored
		.sync_mask_reg = DEFAULT__180305__TPDO4_SYNC_MASK_REGISTER,
		.build_data_option = DEFAULT__180306__TPDO4_BUILD_DATA_OPTION,
	},
    /* TPDO1 Mapping Parameter */
	.x1A0x_tpdo_map_para[0] = {			//<< EDITABLE:
		.object_number	= 0x00,					//<< number of mapped application objects in PDO
		.app_object[0].w	= 0x00000000,			//<< application object 1st
		.app_object[1].w	= 0x00000000,			//<< application object 2st
		.app_object[2].w	= 0x00000000,			//<< application object 3st
		.app_object[3].w	= 0x00000000,			//<< application object 4st
		.app_object[4].w	= 0x00000000,			//<< application object 5st
		.app_object[5].w	= 0x00000000,			//<< application object 6st
		.app_object[6].w	= 0x00000000,			//<< application object 7st
		.app_object[7].w	= 0x00000000			//<< application object 8st
	},

    /* TPDO2 Mapping Parameter */
	.x1A0x_tpdo_map_para[1] = {			//<< EDITABLE:
		.object_number	= 0x00,					//<< number of mapped application objects in PDO
		.app_object[0].w	= 0x00000000,			//<< application object 1st
		.app_object[1].w	= 0x00000000,			//<< application object 2st
		.app_object[2].w	= 0x00000000,			//<< application object 3st
		.app_object[3].w	= 0x00000000,			//<< application object 4st
		.app_object[4].w	= 0x00000000,			//<< application object 5st
		.app_object[5].w	= 0x00000000,			//<< application object 6st
		.app_object[6].w	= 0x00000000,			//<< application object 7st
		.app_object[7].w	= 0x00000000			//<< application object 8st
	},
    /* TPDO3 Mapping Parameter */
	.x1A0x_tpdo_map_para[2] = {			//<< EDITABLE:
		.object_number	= 0x00,					//<< number of mapped application objects in PDO
		.app_object[0].w	= 0x00000000,			//<< application object 1st
		.app_object[1].w	= 0x00000000,			//<< application object 2st
		.app_object[2].w	= 0x00000000,			//<< application object 3st
		.app_object[3].w	= 0x00000000,			//<< application object 4st
		.app_object[4].w	= 0x00000000,			//<< application object 5st
		.app_object[5].w	= 0x00000000,			//<< application object 6st
		.app_object[6].w	= 0x00000000,			//<< application object 7st
		.app_object[7].w	= 0x00000000			//<< application object 8st
	},
    /* TPDO4 Mapping Parameter */
	.x1A0x_tpdo_map_para[3] = {			//<< EDITABLE:
		.object_number	= 0x00,					//<< number of mapped application objects in PDO
		.app_object[0].w	= 0x00000000,			//<< application object 1st
		.app_object[1].w	= 0x00000000,			//<< application object 2st
		.app_object[2].w	= 0x00000000,			//<< application object 3st
		.app_object[3].w	= 0x00000000,			//<< application object 4st
		.app_object[4].w	= 0x00000000,			//<< application object 5st
		.app_object[5].w	= 0x00000000,			//<< application object 6st
		.app_object[6].w	= 0x00000000,			//<< application object 7st
		.app_object[7].w	= 0x00000000			//<< application object 8st
	},
};

