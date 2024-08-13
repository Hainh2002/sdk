/*
 * manu_od_default_value.c
 *
 *  Created on: Oct 12, 2022
 *      Author: Dakaka
 */

#include "manu_od_default_value.h"

/************************************************************************************
 *					User default value for CANOPEN application/manufactor-specific
 ************************************************************************************/

#define DEFAULT__500000__RATED_VOLTAGE_MV						9999
#define DEFAULT__500001__RATED_CURRENT_MV						9999
#define DEFAULT__500100__DEFAULT_BAUD_RATE_KHZ					9999

/*******************************************************************************************
 * Define and assign default value to some application/manufactor-specific profile initializer data variable
 * 				WARNING:
 *				- Edit carefully !!!
 * *******************************************************************************************/

OD_Temp_Manufacturer_Profile_Para_t od_temp_manu_profile_para =
{
		.x500200_rated_volt_mV = DEFAULT__500000__RATED_VOLTAGE_MV,
		.x500201_rated_curr_mA = DEFAULT__500001__RATED_CURRENT_MV,
		.x500300_node_ID = NODE_ID_DEFAULT,
		.x500301_baud_rate_kHz = DEFAULT__500100__DEFAULT_BAUD_RATE_KHZ,
		/* USER CODE 0*/

		/* USER CODE 0*/

};
