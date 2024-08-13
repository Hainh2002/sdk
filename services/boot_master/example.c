//
// Created by vuonglk on 07/05/2024.
//

#include<sm_logger.h>
#include <stdio.h>
#include <sm_sv_boot_master.h>
#include <sm_mb_master_impl.h>
#include <time.h>
#include <pthread.h>


#include "linux_serial.h"
#include "sm_co_if.h"
#include "app_co_init.h"
#include <sm_time_utils.h>
#define TAG "main"

#define TEST_OTA_SLAVE 0
#define TEST_OTA_CAN 1

#define USB_PORT "/dev/ttyUSB0"
#define SLAVE_HEX_PATH "./slave_mainapp.hex"
#define BP_HEX_PATH "./bp_mainapp.hex"

#define CO_TCP_SERVER_IP    "192.168.1.254"
#define CO_TCP_SERVER_PORT  32000

#define SM_DEVICE_BOOT_INDEX                            0x2001
#define SN_DEVICE_REBOOT_SUB_INDEX                      0x07




#define TEST_SLAVE_ID 0

int g_fd;
sm_mb_master_t* g_mb_master = NULL;
sm_sv_boot_master_events_callback_t g_boot_event;
sm_sv_boot_master_t* g_boot_master = NULL;
sm_sv_boot_dev_if_fn_t g_slave_boot_if;
sm_sv_boot_dev_if_fn_t g_canopen_boot_if;

bool g_is_reboot = false;

elapsed_timer_t reboot_timer;



int64_t get_tick_count() {
    struct timespec ts;
    int32_t tick = 0U;
    clock_gettime(CLOCK_REALTIME, &ts);
    tick = ts.tv_nsec / 1000000;
    tick += ts.tv_sec * 1000;
    return tick;
}

void log_put(const char* _log) {
    printf("%s\n", _log);
}

int32_t my_serial_write(const uint8_t* _data, int32_t _len, int32_t _timeout, void* _arg) {
    return serial_send_bytes(g_fd, _data, _len);
}

int32_t my_serial_read(uint8_t* _buf, int32_t _len, int32_t _timeout, void* _arg) {
    return serial_recv_bytes(g_fd, _buf, _len);
}


void on_test_sm_boot_master_callback_reboot(uint8_t _devId, uint8_t _issSuccess, void* _arg) {
    LOG_INF(TAG, "Reboot Device %d %s", _devId, _issSuccess? "SUCCESS" : "FAILED");
    delayMs(200);
}

void on_test_sm_boot_master_callback_on_upgrading_status(uint8_t _devId, const char* _ver, uint8_t _err, void* _arg) {
    LOG_INF(TAG, "Upgrade Device %d with version %s %s", _devId, _ver,(_err == SM_SV_BOOT_MASTER_ERR_NONE)? "SUCCESS" : "FAILED");
}

void on_test_sm_boot_master_callback_finish_upgrading() {
    LOG_INF(TAG, "All device have been upgraded");
}

int on_test_sm_boot_master_reboot_slave(uint8_t _id, uint32_t _timeout, void* _arg) {
     sm_sv_mb_master_write_single_reg(g_mb_master, _id + SLAVE_ID_OFFSET,
                                  MODBUS_RTU_DECODE_REGISTER(BSS_MODBUS_CAB_CTL_STATE), BSS_MODBUS_CAB_STATE_REBOOT);
    elapsed_timer_resetz(&reboot_timer, 5800);
    return 1;
}

bool on_test_sm_boot_master_check_slave_reboot_condition(uint8_t _id, void* _arg) {
    return !elapsed_timer_get_remain(&reboot_timer);
}

bool on_test_sm_boot_master_check_slave_upgrading_condition(uint8_t _id, void* _arg) {
    return true;
}

static uint8_t m_reboot_state;
static uint8_t m_response_ext_reboot = 0;
static CO_Sub_Object_Ext_Confirm_Func_t sm_boot_canopen_response_ext_boot_handle(void){
    return CO_EXT_CONFIRM_success;
}
CO_Sub_Object_Ext_t m_ext_obj;

int on_test_sm_boot_master_reboot_canopen(uint8_t _id, uint32_t _timeout, void* _arg) {
    struct CO_SDO_t* sdoClient = &CO_DEVICE.sdo_client;
    m_reboot_state = 1;

    g_is_reboot = false;

    struct CO_Sub_Object_t subObject = {
            .p_data = &m_reboot_state,
            .attr = ODA_SDO_RW,
            .len = 1,
            .p_ext = &m_ext_obj,
            .p_temp_data = NULL
    };

    LOG_INF(TAG, "Start reboot BP, node ID: %d", _id);

    if(CO_SDO_get_status(sdoClient) == CO_SDO_RT_busy){
        LOG_ERR(TAG, "SDO Status BUSY");
        return -1;
    }

    CO_SDOclient_start_download(sdoClient,
                                0,
                                SM_DEVICE_BOOT_INDEX,
                                SN_DEVICE_REBOOT_SUB_INDEX,
                                &subObject,
                                600);

    return 1;
}

bool on_test_sm_boot_master_check_canopen_upgrading_condition(uint8_t _id, void* _arg) {
    return true;
}

bool on_test_sm_boot_master_check_canopen_reboot_condition(uint8_t _id, void* _arg) {
    return true;
}

void canHandleRecv(uint32_t _can_id, uint8_t* _data, void* _arg){

}

void *myThreadFun(void *_arg)
{
    while (1){
        app_co_process();
    }
}


int main() {
    printf("test boot master\n");
    sm_logger_init(log_put, LOG_LEVEL_DEBUG);

    g_boot_event.finish_upgrading = on_test_sm_boot_master_callback_finish_upgrading;
    g_boot_event.on_upgrading_status = on_test_sm_boot_master_callback_on_upgrading_status;
    g_boot_event.reboot_dev = on_test_sm_boot_master_callback_reboot;

    g_boot_master = sm_sv_boot_master_create(&g_boot_event);
    if (g_boot_master == NULL) {
        printf("cannot create boot master\n");
        return -1;
    }

    sm_sv_boot_master_set_input_if_fn(g_boot_master, SM_SV_BOOT_MASTER_INPUT_FILE, NULL, NULL, NULL);

    if(TEST_OTA_SLAVE){
        sm_sv_boot_master_set_output_if_fn(g_boot_master, SM_SV_BOOT_MASTER_OUTPUT_HOST_SYNC, my_serial_write,
                                           my_serial_read,
                                           NULL);

        g_fd = serial_init(USB_PORT, 115200, SERIAL_FLAG_BLOCKING);
        if (g_fd < 0) {
            printf("cannot open serial port\n");
        return -1;
        }

        g_mb_master = sm_mb_master_create(my_serial_write, my_serial_read, NULL);
        if (g_mb_master == NULL) {
            printf("cannot create mb master\n");
        return -1;
        }

        g_slave_boot_if.reboot_dev = on_test_sm_boot_master_reboot_slave;
        g_slave_boot_if.check_dev_upgrading_condition = on_test_sm_boot_master_check_slave_upgrading_condition;
        g_slave_boot_if.get_dev_reboot_status = on_test_sm_boot_master_check_slave_reboot_condition;
        g_slave_boot_if.arg = NULL;


//        sm_sv_boot_master_request_upgrade(g_boot_master, 0, "2.0.0", SLAVE_HEX_PATH, &g_slave_boot_if, NULL);
        sm_sv_boot_master_request_upgrade(g_boot_master, 1, "2.0.1", SLAVE_HEX_PATH, &g_slave_boot_if, NULL);
//        sm_sv_boot_master_request_upgrade(g_boot_master, 0, "2.0.3", SLAVE_HEX_PATH, &g_slave_boot_if, NULL);
    }

    if(TEST_OTA_CAN){
        sm_co_if_t *coInterface = sm_co_if_create_default(CO_ETHERNET_CANBUS_IF,
                                                          CO_TCP_SERVER_IP,
                                                          CO_TCP_SERVER_PORT,
                                                          NULL);

        app_co_init(coInterface, canHandleRecv, coInterface);

        sm_sv_boot_master_set_output_if_fn(g_boot_master, SM_SV_BOOT_MASTER_OUTPUT_CANOPEN, NULL, NULL, NULL);

        g_canopen_boot_if.check_dev_upgrading_condition = on_test_sm_boot_master_check_canopen_upgrading_condition;
        g_canopen_boot_if.reboot_dev = on_test_sm_boot_master_reboot_canopen;
        g_canopen_boot_if.get_dev_reboot_status = on_test_sm_boot_master_check_canopen_reboot_condition;
        g_canopen_boot_if.arg = NULL;

        sm_sv_boot_master_request_upgrade(g_boot_master, 4, "2.0.1", BP_HEX_PATH, &g_canopen_boot_if, NULL);

//        pthread_t thread_id;
//        pthread_create(&thread_id, NULL, myThreadFun, NULL);
    }

    while (1) {
        sm_sv_boot_master_process(g_boot_master);
        app_co_process();
        CO_SDO_return_t status = CO_SDO_get_status(&CO_DEVICE.sdo_client);
        if(!g_is_reboot){
            if(status == CO_SDO_RT_abort){
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
                LOG_ERR(TAG, "Write SDO FAILED!!!");
            }else if(status == CO_SDO_RT_success) {
                if(CO_DEVICE.sdo_client.m_sub_index == SN_DEVICE_REBOOT_SUB_INDEX){
                    g_is_reboot = true;
                }
                LOG_INF(TAG, "Write SDO SUCCEED!!!");
                CO_SDO_reset_status(&CO_DEVICE.sdo_client);
            }
        }
    }
}
