/*
 * manu_od_default_value.h
 *
 *  Created on: Oct 12, 2022
 *      Author: Dakaka
 */

#ifndef APP_APP_CO_OD_MANU_OD_MANU_OD_DEFAULT_VALUE_H_
#define APP_APP_CO_OD_MANU_OD_MANU_OD_DEFAULT_VALUE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "od_config.h"

/*******************************************************************************************
 * Declare some manufacturer-specific profile initializer data variable
 * *******************************************************************************************/

typedef struct {
    Boot_Setting_t x5000_boot1_setting;    //<< Not save into flash by "store parameter", "restore default parameter" command, not calculate CRC
    Boot_Setting_t x5001_boot2_setting;    //<< Not save into flash by "store parameter", "restore default parameter" command, not calculate CRC

    uint32_t x500200_rated_volt_mV;
    uint32_t x500201_rated_curr_mA;
    uint8_t x500300_node_ID;
    uint16_t x500301_baud_rate_kHz;

    /* USER CODE 1 BEGIN */



    /* USER CODE 1 END */
} OD_Temp_Manufacturer_Profile_Para_t;

extern OD_Temp_Manufacturer_Profile_Para_t od_temp_manu_profile_para;

#ifdef __cplusplus
};
#endif


#endif /* APP_APP_CO_OD_MANU_OD_MANU_OD_DEFAULT_VALUE_H_ */
