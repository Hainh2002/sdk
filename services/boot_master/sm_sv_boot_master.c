//
// Created by vuonglk on 06/05/2024.
//

#include "sm_sv_boot_master.h"
#include "sm_logger.h"
#include "sm_boot_impl.h"
#include "mb_master/sm_mb_master_impl.h"

#define TAG "sm_sv_boot_master"

#define SM_SV_BOOT_MAX_DEV_SUPPORT               64
#define SM_SV_BOOT_WAITING_ONLINE_TIMEOUT       45000
#define SM_SV_BOOT_REBOOT_DEV_TIMEOUT           10000


typedef enum
{
    UPGRADING_DEV_IDLE = 0,
    UPGRADING_DEV_WAITING_ONLINE,
    UPGRADING_DEV_WAITING_REBOOT,
    UPGRADING_DEV_UPGRADING,
} UPGRADING_DEVICE_STATE;

typedef struct sm_sv_boot_master_impl {
    sm_vector_t* m_upgradeDev;
    sm_sv_boot_dev_info_t* m_currentUpgradingDev;
    sm_sv_boot_master_events_callback_t* m_eventCallback;
    elapsed_timer_t m_timeout;
    sm_sv_boot_master_if_fn_t m_output_if;
    sm_sv_boot_master_if_fn_t m_input_if;
    uint8_t m_inputType;
    uint8_t m_outputType;
    sm_boot_master_t* m_bootMaster;
    void* m_hostMaster;
    void* m_modbusMaster;
} sm_sv_boot_master_impl_t;

#define impl(x) ((sm_sv_boot_master_impl_t*)(x))


static void sm_sv_boot_reset_upgrading_dev(sm_sv_boot_master_t* _this, SM_SV_BOOT_MASTER_UPGRADING_ERROR _error);

static int sm_sv_boot_prepare_upgrading_dev(sm_sv_boot_master_t* _this);

static int sm_sv_boot_start_upgrading_dev(sm_sv_boot_master_t* _this);

static void boot_master_event_callback(int32_t _error, int32_t _id, void* _arg) {
    sm_sv_boot_master_impl_t* this = impl(_arg);
    if (this == NULL) return;

    if(!_error){
        LOG_INF(TAG, "Upgrading device %d is SUCCESS", _id);
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_NONE);
    }else{
        LOG_ERR(TAG, "Upgrading device %d is FAILURE, error code of boot-core %d", this->m_currentUpgradingDev->m_devId, _error);
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_DOWNLOAD);
    }
}

static int32_t sm_sv_boot_master_mb_write_multi_regs(void* _modbus_master, uint16_t _id, uint16_t _addr,
                                                     uint16_t _quantity, const uint16_t* _regs) {
    if (_modbus_master == NULL) return MODBUS_ERROR_TRANSPORT;

    return sm_sv_mb_master_write_multi_regs(_modbus_master, _id, _addr, _quantity, _regs);
}

static int32_t sm_sv_boot_master_mb_read_holding_registers(void* _modbus_master, uint16_t _id, uint16_t _addr,
                                                           uint16_t _quantity, uint16_t* _regs) {
    if (_modbus_master == NULL) return MODBUS_ERROR_TRANSPORT;

    return sm_sv_mb_master_read_hold_regs(_modbus_master, _id, _addr, _quantity, _regs);
}


sm_sv_boot_master_t* sm_sv_boot_master_create(sm_sv_boot_master_events_callback_t* _eventCallback) {
    sm_sv_boot_master_impl_t* this = malloc(sizeof(sm_sv_boot_master_impl_t));
    if (this == NULL) return NULL;

    this->m_upgradeDev = sm_vector_create(SM_SV_BOOT_MAX_DEV_SUPPORT, sizeof(sm_sv_boot_dev_info_t));
    if (this->m_upgradeDev == NULL) return NULL;

    this->m_eventCallback = _eventCallback;
    this->m_currentUpgradingDev = NULL;

    this->m_bootMaster = sm_boot_master_create(boot_master_event_callback, this);

    if(this->m_bootMaster == NULL){
        LOG_ERR(TAG, "Create Boot Master FAILURE");
        return NULL;
    }

    return this;
}

int sm_sv_boot_master_set_output_if_fn(sm_sv_boot_master_t* _this, uint8_t _outputType,
                                       sm_boot_master_send_if_fn _sendIf, sm_boot_master_receive_if_fn _recvIf,
                                       void* _arg) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return -1;

    this->m_outputType = _outputType;
    this->m_output_if.send_if = _sendIf;
    this->m_output_if.receive_if = _recvIf;
    this->m_output_if.arg = _arg;

    if (this->m_outputType == SM_SV_BOOT_MASTER_OUTPUT_HOST_SYNC) {
        this->m_hostMaster = sm_host_create_default(SM_HOST_SYNC_MODE, 0x00, this->m_output_if.send_if,
                                                    this->m_output_if.receive_if, this->m_output_if.arg);
        if (this->m_hostMaster == NULL) {
            LOG_ERR(TAG, "Can not create host boot output");
            return -1;
        }
    }
    else if (this->m_outputType == SM_SV_BOOT_MASTER_OUTPUT_MODBUS_RTU) {
        this->m_modbusMaster = sm_mb_master_create(this->m_output_if.send_if, this->m_output_if.receive_if,
                                                   this->m_output_if.arg);
        if (this->m_modbusMaster == NULL) {
            LOG_ERR(TAG, "Can not create modbus RTU boot output");
            return -1;
        }
    }
    return 0;
}

int sm_sv_boot_master_set_input_if_fn(sm_sv_boot_master_t* _this, uint8_t _inputType, sm_boot_master_send_if_fn _sendIf,
                                      sm_boot_master_receive_if_fn _recvIf, void* _arg) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return -1;

    this->m_inputType = _inputType;
    this->m_input_if.send_if = _sendIf;
    this->m_input_if.receive_if = _recvIf;
    this->m_input_if.arg = _arg;

    return 0;
}

int sm_sv_boot_master_request_upgrade(sm_sv_boot_master_t* _this, uint8_t _devId, const char* _ver, const char* _fwPath,
                                      sm_sv_boot_dev_if_fn_t* _if, void* _arg) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return -1;

    sm_sv_boot_dev_info_t devInfo = {.m_devId = _devId, .m_ver = _ver, .m_path = _fwPath, .m_if = _if, .m_arg = _arg};

    if (sm_vector_is_full(this->m_upgradeDev)) {
        return -1;
    }

    devInfo.m_state = UPGRADING_DEV_WAITING_ONLINE;
    elapsed_timer_resetz(&devInfo.m_timeout, SM_SV_BOOT_WAITING_ONLINE_TIMEOUT);
    if (sm_vector_push_back(this->m_upgradeDev, &devInfo) < 0) {
        return -1;
    }
    LOG_INF(TAG, "New device with id %d, version %s is push to queue to upgrading", _devId, _ver);
    return 0;
}

static void sm_sv_boot_reset_upgrading_dev(sm_sv_boot_master_t* _this, SM_SV_BOOT_MASTER_UPGRADING_ERROR _error) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this->m_currentUpgradingDev == NULL) return;
    sm_sv_boot_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

    if (this->m_eventCallback->on_upgrading_status) {
        LOG_INF(TAG, "Notify upgrading error %d of dev %d to Observer", _error, upgradingDev->m_devId);
        this->m_eventCallback->on_upgrading_status(upgradingDev->m_devId,
                                                   upgradingDev->m_ver, _error,
                                                   this->m_eventCallback->arg);
    }

    sm_vector_erase_item(this->m_upgradeDev, upgradingDev);
    this->m_currentUpgradingDev = NULL;

    if (sm_vector_get_size(this->m_upgradeDev) == 0) {
        if (this->m_eventCallback->finish_upgrading) {
            LOG_INF(TAG, "Finished upgrading firmware, Notify to observer");
            this->m_eventCallback->finish_upgrading(this->m_eventCallback->arg);
        }
    }
    else {
        for (int index = 0; index < sm_vector_get_size(this->m_upgradeDev); index++) {
            sm_sv_boot_dev_info_t* dev = sm_vector_get_item(this->m_upgradeDev, index);
            elapsed_timer_resetz(&dev->m_timeout, SM_SV_BOOT_WAITING_ONLINE_TIMEOUT);
            dev->m_state = UPGRADING_DEV_WAITING_ONLINE;
        }
    }
}

static int sm_sv_boot_prepare_upgrading_dev(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this->m_currentUpgradingDev == NULL) return -1;
    sm_sv_boot_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

    LOG_INF(TAG, "Now reboot device id %d", upgradingDev->m_devId);
    if(upgradingDev->m_if->reboot_dev == NULL){
        upgradingDev->m_state = UPGRADING_DEV_UPGRADING;
        sm_sv_boot_start_upgrading_dev(this);
        return 0;
    }

    if (upgradingDev->m_if->reboot_dev(upgradingDev->m_devId, SM_SV_BOOT_REBOOT_DEV_TIMEOUT,
                                                      upgradingDev->m_if->arg) < 0) {
        LOG_ERR(TAG, "Reboot device id %d FAILED", upgradingDev->m_devId);
        this->m_eventCallback->reboot_dev(upgradingDev->m_devId, false, this->m_eventCallback->arg);
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_REBOOT);
        return -1;
    }

    if(upgradingDev->m_if->get_dev_reboot_status == NULL){
        upgradingDev->m_state = UPGRADING_DEV_UPGRADING;
        this->m_eventCallback->reboot_dev(upgradingDev->m_devId, true, this->m_eventCallback->arg);
        sm_sv_boot_start_upgrading_dev(this);
    }else{
        elapsed_timer_resetz(&upgradingDev->m_timeout, SM_SV_BOOT_REBOOT_DEV_TIMEOUT);
        upgradingDev->m_state = UPGRADING_DEV_WAITING_REBOOT;
    }
    return 0;
}

static int sm_sv_boot_start_upgrading_dev(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    sm_sv_boot_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

    LOG_INF(TAG, "Start upgrading device id %d", upgradingDev->m_devId);

    sm_boot_output_if_t* bootOutput = NULL;
    sm_boot_input_if_t* bootInput = NULL;

    if (this->m_inputType == SM_SV_BOOT_MASTER_INPUT_FILE) {
        bootInput = sm_get_file_boot_input(upgradingDev->m_path);
        if (bootInput == NULL)
            LOG_ERR(TAG, "FAILED to create FILE Boot-input");
    }

    if (bootInput == NULL) return -1;

    if (this->m_outputType == SM_SV_BOOT_MASTER_OUTPUT_CANOPEN) {
        bootOutput = sm_get_co_boot_output();
        if (bootOutput == NULL){
            LOG_ERR(TAG, "FAILED to create CANOPEN Boot-output");
        }
    }
    else if (this->m_outputType == SM_SV_BOOT_MASTER_OUTPUT_HOST_SYNC) {
        sm_host_set_addr(this->m_hostMaster, upgradingDev->m_devId);
        bootOutput = sm_get_host_sync_boot_output(this->m_hostMaster, this->m_output_if.arg);
        if (bootOutput == NULL) {
            LOG_ERR(TAG, "FAILED to create HOST-SYNC Boot-output");
            return -1;
        }
    }
    else if (this->m_outputType == SM_SV_BOOT_MASTER_OUTPUT_MODBUS_RTU) {
        bootInput->init();
        static sm_fw_signature_t g_fw_info;
        bootInput->get_fw_info(&g_fw_info);
        bootOutput = sm_get_modbus_rtu_boot_output(&g_fw_info, this->m_modbusMaster,
                                                   sm_sv_boot_master_mb_write_multi_regs,
                                                   sm_sv_boot_master_mb_read_holding_registers);
        if (bootOutput == NULL) {
            LOG_ERR(TAG, "FAILED to create HOST-SYNC Boot-output");
            return -1;
        }
    }

    if (sm_boot_master_add_slave(this->m_bootMaster, upgradingDev->m_devId, bootInput, bootOutput) < 0) {
        LOG_ERR(TAG, "Add Boot slave FAILURE");
        bootOutput->m_proc->free(bootOutput);
        bootInput->free();
        sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_INTERNAL);
        return -1;
    }

    upgradingDev->m_state = UPGRADING_DEV_UPGRADING;
    elapsed_timer_reset(&upgradingDev->m_timeout);
    return 0;
}

void sm_sv_boot_master_process(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return;

    if (sm_vector_get_size(this->m_upgradeDev) > 0 && this->m_currentUpgradingDev == NULL) {
        int size = sm_vector_get_size(this->m_upgradeDev);
        for (int index = 0; index < sm_vector_get_size(this->m_upgradeDev); index++) {
            sm_sv_boot_dev_info_t* dev = sm_vector_get_item(this->m_upgradeDev, index);
            if (dev->m_state == UPGRADING_DEV_WAITING_ONLINE && !elapsed_timer_get_remain(&dev->m_timeout)) {
                LOG_ERR(TAG, "The device %d is TIMEOUT while waiting online", dev->m_devId);
                if (this->m_eventCallback->on_upgrading_status) {
                    this->m_eventCallback->on_upgrading_status(dev->m_devId, dev->m_ver, SM_SV_BOOT_MASTER_ERR_TIMEOUT,
                                                               this->m_eventCallback->arg);
                }
                sm_vector_erase_item(this->m_upgradeDev, dev);

                if (this->m_eventCallback->finish_upgrading != NULL && sm_vector_get_size(this->m_upgradeDev) == 0) {
                    LOG_INF(TAG, "Finished upgrading firmware, Notify to observer");
                    this->m_eventCallback->finish_upgrading(this->m_eventCallback->arg);
                }
                return;
            }

            if (dev->m_if->check_dev_upgrading_condition(dev->m_devId, dev->m_if->arg)) {
                this->m_currentUpgradingDev = dev;
                LOG_INF(TAG, "Device %d is enough condition to upgrading", dev->m_devId);
                sm_sv_boot_prepare_upgrading_dev(this);
                return;
            }
        }
    }

    if (this->m_currentUpgradingDev) {
        sm_sv_boot_dev_info_t* upgradingDev = this->m_currentUpgradingDev;

        if (upgradingDev->m_state == UPGRADING_DEV_WAITING_REBOOT){
            if(!elapsed_timer_get_remain(&upgradingDev->m_timeout)){
                this->m_eventCallback->reboot_dev(upgradingDev->m_devId, false, this->m_eventCallback->arg);
                sm_sv_boot_reset_upgrading_dev(this, SM_SV_BOOT_MASTER_ERR_REBOOT);
                return;
            }
            if(upgradingDev->m_if->get_dev_reboot_status(upgradingDev->m_devId, upgradingDev->m_if->arg)){
                this->m_eventCallback->reboot_dev(upgradingDev->m_devId, true, this->m_eventCallback->arg);
                sm_sv_boot_start_upgrading_dev(this);
            }
        }

        if(this->m_currentUpgradingDev->m_state == UPGRADING_DEV_UPGRADING){
            sm_boot_master_process(this->m_bootMaster);
        }
    }
}

int sm_sv_boot_master_destroy(sm_sv_boot_master_t* _this) {
    sm_sv_boot_master_impl_t* this = impl(_this);
    if (this == NULL) return -1;

    sm_vector_destroy(this->m_upgradeDev);
    free(this);
    return 0;
}

#define TAG "sm_sv_boot_master"
