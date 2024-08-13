/*
 * app_co_od_config.h
 *
 *  Created on: Oct 8, 2022
 *      Author: Dakaka
 */

#include "CO.h"

/* Include header file contains CANopen object to here ----------*/
//#include

#define	NODE_ID_DEFAULT												3
#define SW_VERSION													0x000003	//<< Use 3 byte
#define IS_MAIN_APP													1			//<< is main_app: set 1,
																				//<< is boot_app: set 0

#define SEGMENT_MEMORY_SIZE                 				1024        //Not edit!

#define SDO_BOOTLOADER_INDEX                				(0x2001)    //Not edit!
#define SDO_BOOTLOADER_FW_VERSION_SUB_INDEX 				(0x00)      //Not edit!
#define SDO_BOOTLOADER_FW_SIZE_SUB_INDEX    				(0x01)      //Not edit!
#define SDO_BOOTLOADER_FW_CRC_SUB_INDEX     				(0x02)      //Not edit!
#define SDO_BOOTLOADER_SEG_ADDR_SUB_INDEX   				(0x03)      //Not edit!
#define SDO_BOOTLOADER_SEG_DATA_SUB_INDEX   				(0x04)      //Not edit!
#define SDO_BOOTLOADER_SEG_CRC_SUB_INDEX    				(0x05)      //Not edit!
#define SDO_BOOTLOADER_BOOT_STATE_SUB_INDEX 				(0x06)      //Not edit!
#define SDO_BOOTLOADER_BOOT_EXT_REQ_SUB_INDEX   			(0x07)		//Not edit!

#define BOOT_MASTER_NODE_ID                 				(0x03)

#define BP_VOL_CUR_TPDO_COBID                           CO_CAN_ID_TPDO_1
#define BP_CELLS_VOL_1_TO_4         		            CO_CAN_ID_TPDO_2
#define BP_CELLS_VOL_5_TO_8			                    CO_CAN_ID_TPDO_3
#define BP_CELLS_VOL_9_TO_12							CO_CAN_ID_RPDO_1
#define BP_CELLS_VOL_13_TO_16							CO_CAN_ID_RPDO_2
#define BP_TEMP_TPDO_COBID                              CO_CAN_ID_TPDO_4
#define BP_SOH_CYCLE_COBID                              CO_CAN_ID_RPDO_3

extern CO CO_DEVICE;

/* OD STORAGE option : enable (set 1), disable (set 0) */
#define APP_OD_STORAGE__USE											0

#if APP_OD_STORAGE__USE
#define APP_OD_STORAGE__STORAGE_REGION__ADDR						(xxx)
#define APP_OD_STORAGE__STORAGE_REGION__SIZE						(xxx)
#endif
