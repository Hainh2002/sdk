/*
 * comm_od.h
 *
 *  Created on: Oct 12, 2022
 *      Author: Dakaka
 */

#ifndef APP_APP_CO_OD_COMM_OD_COMM_OD_H_
#define APP_APP_CO_OD_COMM_OD_COMM_OD_H_

#ifdef __cplusplus
extern "C"{
#endif


/*
 *		========================================
 *      ============== DON'T EDIT ==============
 *      ========================================
 */
#include "comm_od_default_value.h"

/*******************************************************************************************
 * Declare all communication profile initializer sub-object
 * *******************************************************************************************/

extern CO_Sub_Object obj_1000_device_type[];
extern CO_Sub_Object obj_1001_error_reg[];
extern CO_Sub_Object obj_1002_status[];;
extern CO_Sub_Object obj_1003_error_field[];
extern CO_Sub_Object obj_1005_sync_cob_id[];
extern CO_Sub_Object obj_1006_comm_cycle[];
extern CO_Sub_Object obj_1007_sync_window_len[];
extern CO_Sub_Object obj_1008_device_name[];
extern CO_Sub_Object obj_1009_hw_version[];
extern CO_Sub_Object obj_100A_sw_version[];
extern CO_Sub_Object obj_100C_guard_time[];
extern CO_Sub_Object obj_100D_life_time_factor[];
extern CO_Sub_Object obj_1010_store_para[];
extern CO_Sub_Object obj_1011_restore_default_para[];
extern CO_Sub_Object obj_1012_time_cob_id[];
extern CO_Sub_Object obj_1013_high_res_timestamp[];
extern CO_Sub_Object obj_1014_emcy_cob_id[];
extern CO_Sub_Object obj_1015_inhibit_time_emcy[];
extern CO_Sub_Object obj_1016_consumer_heartbeat[];
extern CO_Sub_Object obj_1017_producer_heartbeat[];
extern CO_Sub_Object obj_1018_identity[];
extern CO_Sub_Object obj_1019_sync_cter_over_flow_value[];
extern CO_Sub_Object obj_1030_sync_bit_pos_over_flow_value[];
extern CO_Sub_Object obj_1031_sdo_server_comm_para[];
extern CO_Sub_Object obj_1032_sdo_client_comm_para[];

#ifdef __cplusplus
};
#endif


#endif /* APP_APP_CO_OD_COMM_OD_COMM_OD_H_ */
