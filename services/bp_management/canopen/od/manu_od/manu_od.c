/*
 * app_co_manu_od.c
 *
 *  Created on: Jul 15, 2022
 *      Author: Dakaka
 *
 */

#include "manu_od.h"

//#include "app_acc_profile.h"

/*******************************************************************************************
 * Define all application/manufactor-specific profile initializer sub-object
 * *******************************************************************************************/

/*--- Object 0x5000: Basic electric specifications ----------------------------*/
CO_Sub_Object obj_5000_basic_elec_specs[] = {
		{(void*)&od_temp_manu_profile_para.x500200_rated_volt_mV, ODA_SDO_R, 4, NULL, NULL},
		{(void*)&od_temp_manu_profile_para.x500201_rated_curr_mA, ODA_SDO_R, 4, NULL, NULL},
};

/*--- Object 0x5001: Basic CANopen node parameters ----------------------------*/
CO_Sub_Object obj_5001_basic_co_node_para[] = {
		{(void*)&od_temp_manu_profile_para.x500300_node_ID, ODA_SDO_RW|ODA_VWTE_after_reset, 1, NULL, (void*)&od_temp_manu_profile_para.x500300_node_ID},
		{(void*)&od_temp_manu_profile_para.x500301_baud_rate_kHz, ODA_SDO_RW|ODA_VWTE_after_reset, 2, NULL, (void*)&od_temp_manu_profile_para.x500301_baud_rate_kHz},
};

/* USER CODE 0 begin */

/* USER CODE 0 end */
