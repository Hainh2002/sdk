/*
 * app_co_comm_od.c
 *
 *  Created on: Jul 15, 2022
 *      Author: Dakaka
 */
/*
 *		========================================
 *      ============== DON'T EDIT ==============
 *      ========================================
 */

#include "comm_od.h"
/*******************************************************************************************
 * Define all communication profile initializer sub-object
 * *******************************************************************************************/

/*--- Object 0x1000: Device type----------------------------*/
CO_Sub_Object obj_1000_device_type[] = {
		{(void*)&CO_DEVICE.device_type, ODA_SDO_R, 4, NULL, (void*)&od_temp_comm_profile_para.x1000_device_type}
};

/*--- Object 0x1001: Error register ----------------------------*/
CO_Sub_Object obj_1001_error_reg[] = {
		{(void*)&CO_DEVICE.emcy.error_reg.std, ODA_SDO_R, 1, NULL, NULL}
};

/*--- Object 0x1002: Manufacturer status register ----------------------------*/
CO_Sub_Object obj_1002_status[] = {
		{(void*)&CO_DEVICE.status, ODA_SDO_R|ODA_TPDO, 4, NULL, NULL}
};

/*--- Object 0x1003: Error field ----------------------------*/
CO_Sub_Object obj_1003_error_field[]={
	{(void*)&CO_DEVICE.emcy.predefined_error_fields.error_no, ODA_SDO_R|ODA_TPDO, 1, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[0].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[1].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[2].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[3].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[4].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[5].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[6].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL},
	{(void*)&(CO_DEVICE.emcy.predefined_error_fields.error_code[7].w), ODA_SDO_R|ODA_TPDO, 4, NULL, NULL}
};

/*--- Object 0x1005: COB-ID Sync message ----------------------------*/
CO_Sub_Object obj_1005_sync_cob_id[]={
		{(void*)&CO_DEVICE.sync.msg.id, ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, (void*)&od_temp_comm_profile_para.x1005_cob_id_sync_message}
};

/*--- Object 0x1006: Communication cycle period [ms]----------------------------*/
CO_Sub_Object obj_1006_comm_cycle[]={
		{(void*)&CO_DEVICE.sync.cycle_period_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, NULL, (void*)&od_temp_comm_profile_para.x1006_comm_cycle_period_ms}
};

/*--- Object 0x1007: Synchronous window length ----------------------------*/
CO_Sub_Object obj_1007_sync_window_len[]={
		{(void*)&CO_DEVICE.sync.window_length_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, NULL, (void*)&od_temp_comm_profile_para.x1007_sync_window_length_ms}
};

/*--- Object 0x1008: Manufacturer device name ----------------------------*/
CO_Sub_Object obj_1008_device_name[]={
		{(void*)&od_temp_comm_profile_para.x1008_device_name,ODA_SDO_R,12,NULL,NULL}
};

/*--- Object 0x1009: Manufacturer hardware version----------------------------*/
CO_Sub_Object obj_1009_hw_version[]={
		{(void*)&od_temp_comm_profile_para.x1009_hardware_version, ODA_SDO_R, 4, NULL, NULL}
};

/*--- Object 0x100A: Manufacturer software version----------------------------*/
CO_Sub_Object obj_100A_sw_version[]={
		{(void*)&od_temp_comm_profile_para.x100A_software_version, ODA_SDO_R, 4, NULL, NULL}
};

/*--- Object 0x100C:Guard time [ms] ----------------------------*/
CO_Sub_Object obj_100C_guard_time[]={
		{(void*)&od_temp_comm_profile_para.x100C_guard_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, NULL, NULL}
};

/*--- Object 0x100D:Life time factor ----------------------------*/
CO_Sub_Object obj_100D_life_time_factor[]={
		{(void*)&od_temp_comm_profile_para.x100D_life_time_factor, ODA_SDO_RW|ODA_VWTE_right_away, 1, NULL, NULL}
};

/*--- Object 1010h: Store parameters ----------------------------*/

static uint32_t storage_buffer = {0};

/* Sub_Object 00h: Store all parameters */
static CO_Sub_Object_Ext_Confirm_Func_t confirm_func_0x101000_check_validity_of_store_all_para_command_impl(void)
{
	CO_OD_Storage* p_storage = &CO_DEVICE.storage;
	if (storage_buffer == OD_1010_VALID_STORE_COMMAND)
	{
		if(
		/* Save parameter on RAM into FLASH */
				CO_OD_STORAGE_save_comm_para_to_flash(p_storage) == -1
				|| CO_OD_STORAGE_save_app_para_to_flash(p_storage) == -1)
		{
		/* Handle when save fail */
			CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__COMM_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
			CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__APP_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
			if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
			{
				CO_FLASH_error_handle(&CO_DEVICE.flash);
			}
			CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_HW);
			return CO_EXT_CONFIRM_abort;
		}
		/* Handle when save successfully */
		CO_OD_STORAGE_update_crc16_comm_from_data_in_ram(p_storage);
		CO_OD_STORAGE_update_crc16_app_from_data_in_ram(p_storage);
		if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
		{
			CO_FLASH_error_handle(&CO_DEVICE.flash);
		}
		return CO_EXT_CONFIRM_success;
	}
	CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_DATA_TRANSF);
	return CO_EXT_CONFIRM_abort;
}
CO_Sub_Object_Ext_t sub_obj_ext_0x101000 =
{
		.p_shadow_data = (uint8_t*)&storage_buffer,
		.confirm_func = confirm_func_0x101000_check_validity_of_store_all_para_command_impl
};
/* Sub_Object 01h: Store communication parameters */
static CO_Sub_Object_Ext_Confirm_Func_t confirm_func_0x101001_check_validity_of_store_comm_para_command_impl(void)
{
	CO_OD_Storage* p_storage = &CO_DEVICE.storage;
	if (storage_buffer == OD_1010_VALID_STORE_COMMAND)
	{
		if(
		/* Save parameter on RAM into FLASH */
				CO_OD_STORAGE_save_comm_para_to_flash(p_storage) == -1)
		{
		/* Handle when save fail */
			CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__COMM_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
			CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__APP_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
			if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
			{
				CO_FLASH_error_handle(&CO_DEVICE.flash);
			}
			CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_HW);
			return CO_EXT_CONFIRM_abort;
		}
		/* Handle when save successfully */
		CO_OD_STORAGE_update_crc16_comm_from_data_in_ram(p_storage);
		if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
		{
			CO_FLASH_error_handle(&CO_DEVICE.flash);
		}

		return CO_EXT_CONFIRM_success;
	}
	CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_DATA_TRANSF);
	return CO_EXT_CONFIRM_abort;
}
CO_Sub_Object_Ext_t sub_obj_ext_0x101001 =
{
		.p_shadow_data = (uint8_t*)&storage_buffer,
		.confirm_func = confirm_func_0x101001_check_validity_of_store_comm_para_command_impl
};
/* Sub_Object 02h: Store application parameters */
static CO_Sub_Object_Ext_Confirm_Func_t confirm_func_0x101002_check_validity_of_store_app_para_command_impl(void)
{
	CO_OD_Storage* p_storage = &CO_DEVICE.storage;
	if (storage_buffer == OD_1010_VALID_STORE_COMMAND)
	{
		if(
		/* Save parameter on RAM into FLASH */
				CO_OD_STORAGE_save_app_para_to_flash(p_storage) == -1)
		{
		/* Handle when save fail */
			CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__COMM_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
			CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__APP_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
			if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
			{
				CO_FLASH_error_handle(&CO_DEVICE.flash);
			}
			CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_HW);
			return CO_EXT_CONFIRM_abort;
		}
		/* Handle when save successfully */
		CO_OD_STORAGE_update_crc16_app_from_data_in_ram(p_storage);
		if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
		{
			CO_FLASH_error_handle(&CO_DEVICE.flash);
		}
		return CO_EXT_CONFIRM_success;
	}
	CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_DATA_TRANSF);
	return CO_EXT_CONFIRM_abort;
}
CO_Sub_Object_Ext_t sub_obj_ext_0x101002 =
{
		.p_shadow_data = (uint8_t*)&storage_buffer,
		.confirm_func = confirm_func_0x101002_check_validity_of_store_app_para_command_impl
};
/* Object */
CO_Sub_Object obj_1010_store_para[] =
{
		{(void*)&storage_buffer, ODA_SDO_W|ODA_VWTE_right_away, 4, &sub_obj_ext_0x101000, NULL},
		{(void*)&storage_buffer, ODA_SDO_W|ODA_VWTE_right_away, 4, &sub_obj_ext_0x101001, NULL},
		{(void*)&storage_buffer, ODA_SDO_W|ODA_VWTE_right_away, 4, &sub_obj_ext_0x101002, NULL},
};

/*--- Object 1011h: Restore parameters ----------------------------*/

/* Sub_Object 00h: Restore all parameters */
static CO_Sub_Object_Ext_Confirm_Func_t confirm_func_0x101100_check_validity_of_restore_all_para_command_impl(void)
{
	CO_OD_Storage* p_storage = &CO_DEVICE.storage;
	if (storage_buffer == OD_1011_VALID_RESTORE_COMMAND)
	{
		CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__COMM_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
		CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__APP_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
		if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
		{
			CO_FLASH_error_handle(&CO_DEVICE.flash);
			CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_HW);
			return CO_EXT_CONFIRM_abort;
		}
		return CO_EXT_CONFIRM_success;
	}
	CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_DATA_TRANSF);
	return CO_EXT_CONFIRM_abort;
}
CO_Sub_Object_Ext_t sub_obj_ext_0x101100 =
{
		.p_shadow_data = (uint8_t*)&storage_buffer,
		.confirm_func = confirm_func_0x101100_check_validity_of_restore_all_para_command_impl
};
/* Sub_Object 01h: Restore communication parameters */
static CO_Sub_Object_Ext_Confirm_Func_t confirm_func_0x101101_check_validity_of_restore_comm_para_command_impl(void)
{
	CO_OD_Storage* p_storage = &CO_DEVICE.storage;
	if (storage_buffer == OD_1011_VALID_RESTORE_COMMAND)
	{
		CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__COMM_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
		if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
		{
			CO_FLASH_error_handle(&CO_DEVICE.flash);
			CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_HW);
			return CO_EXT_CONFIRM_abort;
		}
		return CO_EXT_CONFIRM_success;
	}
	CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_DATA_TRANSF);
	return CO_EXT_CONFIRM_abort;
}
CO_Sub_Object_Ext_t sub_obj_ext_0x101101 =
{
		.p_shadow_data = (uint8_t*)&storage_buffer,
		.confirm_func = confirm_func_0x101101_check_validity_of_restore_comm_para_command_impl
};
/* Sub_Object 02h: Restore application parameters */
static CO_Sub_Object_Ext_Confirm_Func_t confirm_func_0x101102_check_validity_of_restore_app_para_command_impl(void)
{
	CO_OD_Storage* p_storage = &CO_DEVICE.storage;
	if (storage_buffer == OD_1011_VALID_RESTORE_COMMAND)
	{
		CO_OD_SIGN_set_func(&p_storage->sign, CO_FCG__APP_PARA, CO_OD_SIGN__FUNC_CODE_COMMAND__RESTORE_PARA);
		if(CO_OD_STORAGE_save_sign_to_flash(p_storage) == -1)
		{
			CO_FLASH_error_handle(&CO_DEVICE.flash);
			CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_HW);
			return CO_EXT_CONFIRM_abort;
		}
		return CO_EXT_CONFIRM_success;
	}
	CO_SDO_set_tx_abort_code(&CO_DEVICE.sdo_server, CO_SDO_AB_DATA_TRANSF);
	return CO_EXT_CONFIRM_abort;
}
CO_Sub_Object_Ext_t sub_obj_ext_0x101102 =
{
		.p_shadow_data = (uint8_t*)&storage_buffer,
		.confirm_func = confirm_func_0x101102_check_validity_of_restore_app_para_command_impl
};
/* Object */
CO_Sub_Object obj_1011_restore_default_para[] =
{
		{(void*)&storage_buffer, ODA_SDO_W|ODA_VWTE_right_away, 4, &sub_obj_ext_0x101100, NULL},
		{(void*)&storage_buffer, ODA_SDO_W|ODA_VWTE_right_away, 4, &sub_obj_ext_0x101101, NULL},
		{(void*)&storage_buffer, ODA_SDO_W|ODA_VWTE_right_away, 4, &sub_obj_ext_0x101102, NULL},
};




/*--- Object 1012h: COB-ID TIME ----------------------------*/
CO_Sub_Object obj_1012_time_cob_id[]={
		{(void*)&od_temp_comm_profile_para.x1012_cob_id_time_stamp_object, ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, NULL}
};

/*--- Object 1013h: High resolution time stamp ----------------------------*/
CO_Sub_Object obj_1013_high_res_timestamp[]={
		{(void*)&CO_DEVICE.high_resolution_timestamp, ODA_SDO_RW|ODA_VWTE_right_away, 4, NULL, NULL}
};

/*--- Object 1014h: COB-ID EMCY ----------------------------*/
CO_Sub_Object obj_1014_emcy_cob_id[]={
		{(void*)&od_temp_comm_profile_para.x1014_cob_id_emcy, ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, (void*)&od_temp_comm_profile_para.x1014_cob_id_emcy}
		///must fix
};

/*--- Object 1015h: Inhibit time EMCY ----------------------------*/
CO_Sub_Object obj_1015_inhibit_time_emcy[]={
		{(void*)&od_temp_comm_profile_para.x1015_inhibit_time_emcy, ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, (void*)&od_temp_comm_profile_para.x1014_cob_id_emcy}
		///must fix
};

/*--- Object 1016h: Consumer heart-beat time ----------------------------*/
CO_Sub_Object obj_1016_consumer_heartbeat[]={
		{(void*)&od_temp_comm_profile_para.x1016_consumer_heartbeat_time_sub0, ODA_SDO_R, 1, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1016_consumer_heartbeat_time[0], ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1016_consumer_heartbeat_time[1], ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1016_consumer_heartbeat_time[2], ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1016_consumer_heartbeat_time[3], ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1016_consumer_heartbeat_time[4], ODA_SDO_RW|ODA_VWTE_after_reset, 4, NULL, NULL}
};

/*--- Object 1017h: Producer heart-beat time ----------------------------*/
CO_Sub_Object obj_1017_producer_heartbeat[]={
		{(void*)&od_temp_comm_profile_para.x1017_producer_heartbeat_time, ODA_SDO_RW|ODA_VWTE_after_reset, 2, NULL, NULL}
};

/*--- Object 1018h Identify ----------------------------*/
CO_Sub_Object obj_1018_identity[]={
		{(void*)&od_temp_comm_profile_para.x1018_identity.vendor_id, ODA_SDO_RW|ODA_VWTE_right_away, 4, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1018_identity.product_code, ODA_SDO_RW|ODA_VWTE_right_away, 4, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1018_identity.revision_number, ODA_SDO_RW|ODA_VWTE_right_away, 4, NULL, NULL},
		{(void*)&od_temp_comm_profile_para.x1018_identity.serial_number, ODA_SDO_RW|ODA_VWTE_right_away, 4, NULL, NULL}
};

/*--- Object 1019h: Synchronous counter overflow value [1-30000]  ----------------------------*/
CO_Sub_Object obj_1019_sync_cter_over_flow_value[]={
		{(void*)&CO_DEVICE.sync.max_sync_counter, ODA_SDO_RW|ODA_VWTE_right_away, 2, NULL, (void*)&od_temp_comm_profile_para.x1019_sync_cter_over_flow_value}
};

/*--- Object 1030h Synchronous bit position overflow value [0-31] ----------------------------*/
CO_Sub_Object obj_1030_sync_bit_pos_over_flow_value[]={
		{(void*)&CO_DEVICE.sync.max_aux_sync_counter, ODA_SDO_RW|ODA_VWTE_right_away, 1, NULL, (void*)&od_temp_comm_profile_para.x1030_sync_bit_pos_over_flow_value}
};

/*--- Object 1031h SDO server communication parameter ----------------------------*/
CO_Sub_Object obj_1031_sdo_server_comm_para[]={
		{(void*)&od_temp_comm_profile_para.x1031_sdo_server_comm_para.node_id, ODA_SDO_R, 1, NULL, NULL},
		{(void*)&CO_DEVICE.sdo_server.trans_type, ODA_SDO_RW|ODA_VWTE_right_away, 2, NULL, (void*)&od_temp_comm_profile_para.x1031_sdo_server_comm_para.trans_type},
		{(void*)&CO_DEVICE.sdo_server.allowed_timeout_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, NULL, (void*)&od_temp_comm_profile_para.x1031_sdo_server_comm_para.allow_timeout_ms}
};

/*--- Object 1032h SDO client communication parameter ----------------------------*/
CO_Sub_Object obj_1032_sdo_client_comm_para[]={
		{(void*)&CO_DEVICE.sdo_client.trans_type, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x1032_sdo_client_comm_para.trans_type},
		{(void*)&CO_DEVICE.sdo_client.sync_mask_reg, ODA_SDO_RW|ODA_VWTE_right_away, 4, (void*)&od_temp_comm_profile_para.x1032_sdo_client_comm_para.sync_mask_reg}
};

/*--- Object 1800h TPDO1 communication parameter ----------------------------*/
CO_Sub_Object obj_1800_tpdo1_comm_para[]={
		{(void*)&CO_DEVICE.tpdos[0].comm_parameter.id, ODA_SDO_RW|ODA_VWTE_right_away, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[0].id},
		{(void*)&CO_DEVICE.tpdos[0].comm_parameter.transmission_type, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[0].transmission_type},
		{(void*)&CO_DEVICE.tpdos[0].comm_parameter.max_inhibit_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[0].max_inhibit_time_ms},
		{(void*)&CO_DEVICE.tpdos[0].comm_parameter.max_event_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[0].max_event_time_ms},
		{(void*)&CO_DEVICE.tpdos[0].comm_parameter.sync_start_value, ODA_SDO_RW|ODA_VWTE_right_away, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[0].sync_start_value},
		{(void*)&CO_DEVICE.tpdos[0].comm_parameter.sync_mask_reg, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[0].sync_mask_reg},
		{(void*)&CO_DEVICE.tpdos[0].comm_parameter.build_data_option, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[0].build_data_option},
};

/*--- Object 1801h TPDO2 communication parameter ----------------------------*/
CO_Sub_Object obj_1801_tpdo2_comm_para[]={
		{(void*)&CO_DEVICE.tpdos[1].comm_parameter.id, ODA_SDO_RW|ODA_VWTE_right_away, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[1].id},
		{(void*)&CO_DEVICE.tpdos[1].comm_parameter.transmission_type, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[1].transmission_type},
		{(void*)&CO_DEVICE.tpdos[1].comm_parameter.max_inhibit_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[1].max_inhibit_time_ms},
		{(void*)&CO_DEVICE.tpdos[1].comm_parameter.max_event_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[1].max_event_time_ms},
		{(void*)&CO_DEVICE.tpdos[1].comm_parameter.sync_start_value, ODA_SDO_RW|ODA_VWTE_right_away, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[1].sync_start_value},
		{(void*)&CO_DEVICE.tpdos[1].comm_parameter.sync_mask_reg, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[1].sync_mask_reg},
		{(void*)&CO_DEVICE.tpdos[1].comm_parameter.build_data_option, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[1].build_data_option},
};

/*--- Object 1802h TPDO3 communication parameter ----------------------------*/
CO_Sub_Object obj_1802_tpdo3_comm_para[]={
		{(void*)&CO_DEVICE.tpdos[2].comm_parameter.id, ODA_SDO_RW|ODA_VWTE_right_away, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[2].id},
		{(void*)&CO_DEVICE.tpdos[2].comm_parameter.transmission_type, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[2].transmission_type},
		{(void*)&CO_DEVICE.tpdos[2].comm_parameter.max_inhibit_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[2].max_inhibit_time_ms},
		{(void*)&CO_DEVICE.tpdos[2].comm_parameter.max_event_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[2].max_event_time_ms},
		{(void*)&CO_DEVICE.tpdos[2].comm_parameter.sync_start_value, ODA_SDO_RW|ODA_VWTE_right_away, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[2].sync_start_value},
		{(void*)&CO_DEVICE.tpdos[2].comm_parameter.sync_mask_reg, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[2].sync_mask_reg},
		{(void*)&CO_DEVICE.tpdos[2].comm_parameter.build_data_option, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[2].build_data_option},
};

/*--- Object 1803h TPDO4 communication parameter ----------------------------*/
CO_Sub_Object obj_1803_tpdo4_comm_para[]={
		{(void*)&CO_DEVICE.tpdos[3].comm_parameter.id, ODA_SDO_RW|ODA_VWTE_right_away, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[3].id},
		{(void*)&CO_DEVICE.tpdos[3].comm_parameter.transmission_type, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[3].transmission_type},
		{(void*)&CO_DEVICE.tpdos[3].comm_parameter.max_inhibit_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[3].max_inhibit_time_ms},
		{(void*)&CO_DEVICE.tpdos[3].comm_parameter.max_event_time_ms, ODA_SDO_RW|ODA_VWTE_right_away, 2, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[3].max_event_time_ms},
		{(void*)&CO_DEVICE.tpdos[3].comm_parameter.sync_start_value, ODA_SDO_RW|ODA_VWTE_right_away, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[3].sync_start_value},
		{(void*)&CO_DEVICE.tpdos[3].comm_parameter.sync_mask_reg, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[3].sync_mask_reg},
		{(void*)&CO_DEVICE.tpdos[3].comm_parameter.build_data_option, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x180x_tpdo_comm_para[3].build_data_option},
};

/*--- Object 1A00h TPDO1 mapping parameter ----------------------------*/
CO_Sub_Object obj_1A00_tpdo1_mapping_para[]={
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].object_number, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].object_number},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[0].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[0].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[1].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[1].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[2].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[2].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[3].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[3].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[4].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[4].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[5].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[5].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[6].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[6].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[7].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[0].app_object[7].w},
};
/*--- Object 1A01h TPDO2 mapping parameter ----------------------------*/
CO_Sub_Object obj_1A01_tpdo2_mapping_para[]={
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].object_number, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].object_number},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[0].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[0].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[1].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[1].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[2].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[2].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[3].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[3].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[4].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[4].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[5].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[5].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[6].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[6].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[7].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[1].app_object[7].w},
};
/*--- Object 1A02h TPDO3 mapping parameter ----------------------------*/
CO_Sub_Object obj_1A02_tpdo3_mapping_para[]={
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].object_number, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].object_number},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[0].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[0].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[1].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[1].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[2].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[2].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[3].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[3].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[4].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[4].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[5].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[5].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[6].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[6].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[7].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[2].app_object[7].w},
};
/*--- Object 1A03h TPDO4 mapping parameter ----------------------------*/
CO_Sub_Object obj_1A03_tpdo4_mapping_para[]={
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].object_number, ODA_SDO_RW|ODA_VWTE_after_reset, 1, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].object_number},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[0].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[0].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[1].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[1].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[2].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[2].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[3].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[3].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[4].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[4].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[5].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[5].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[6].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[6].w},
		{(void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[7].w, ODA_SDO_RW|ODA_VWTE_after_reset, 4, (void*)&od_temp_comm_profile_para.x1A0x_tpdo_map_para[3].app_object[7].w},
};
