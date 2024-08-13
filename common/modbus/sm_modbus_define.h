//
// Created by mrgundam on 06/05/2024.
//

#ifndef SM_MODBUS_DEFINE_H
#define SM_MODBUS_DEFINE_H

#define MODBUS_MAX_COILS_SUPPORT    250

#define MODBUS_RESPONSE_TIMEOUT_DEFAULT 200

#define BSS_MODBUS_COM_BAUD_RATE_DEFAULT    115200

#define SLAVE_ID_OFFSET                     1

enum MODBUS_FUNCTION_CODE{
    READ_COILS                          = 0x01,
    READ_DISCRETE_INPUT                 = 0x02,
    READ_HOLDING_REGISTERS              = 0x03,
    READ_INPUT_REGISTERS                = 0x04,
    WRITE_SINGLE_COIL                   = 0x05,
    WRITE_SINGLE_HOLDING_REGISTER       = 0x06,
    WRITE_MULTIPLE_COILS                = 0x15,
    WRITE_MULTIPLE_HOLDING_REGISTERS    = 0x16,
    READ_FILE_RECORD                    = 0x14,
    WRITE_FILE_RECORD                   = 0x15
};

typedef enum {
    // Library errors
    MODBUS_ERROR_INVALID_UNIT_ID = -7,  /**< Received invalid unit ID in response from server */
    MODBUS_ERROR_INVALID_TCP_MBAP = -6, /**< Received invalid TCP MBAP */
    MODBUS_ERROR_CRC = -5,              /**< Received invalid CRC */
    MODBUS_ERROR_TRANSPORT = -4,        /**< Transport error */
    MODBUS_ERROR_TIMEOUT = -3,          /**< Read/write m_timeout occurred */
    MODBUS_ERROR_INVALID_RESPONSE = -2, /**< Received invalid response from server */
    MODBUS_ERROR_INVALID_ARGUMENT = -1, /**< Invalid argument provided */
    MODBUS_ERROR_NONE = 0,              /**< No error */

    // Modbus exceptions
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 1,      /**< Modbus exception 1 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS = 2,  /**< Modbus exception 2 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE = 3,    /**< Modbus exception 3 */
    MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE = 4, /**< Modbus exception 4 */
}MODBUS_ERROR_CODE;

#define MODBUS_RTU_ENCODE_ID(code,reg)               ((((uint32_t)code) << 16) | (reg+1))
#define MODBUS_RTU_DECODE_FUNCTION_CODE(value)       ((uint8_t)((value)>>16 & 0xFF))
#define MODBUS_RTU_DECODE_REGISTER(value)            ((uint16_t)(((value) & 0xFFFF) - 1))

/// DECODE register Value;
#define MODBUS_RTU_REG_GET_HIGH_BYTE(value)         (((value) >> 8) & 0xFF)
#define MODBUS_RTU_REG_GET_LOW_BYTE(value)          ((value) & 0xFF)

#define BSS_MODBUS_CAB_STATUS_TEMP              0x40001
#define BSS_MODBUS_CAB_FAN_STT_INDEX                 0
#define BSS_MODBUS_CAB_CHARGER_SWITCH_STT_INDEX      1
#define BSS_MODBUS_CAB_DOOR_STT_INDEX                2

#define BSS_MODBUS_CAB_POGO_TEMP_12             0x40002
#define BSS_MODBUS_CAB_POGO_TEMP_34             0x40003

#define BSS_MODBUS_CAB_SW_VERSION               0x40060
#define BSS_MODBUS_CAB_HW_VERSION               0x40062
#define BSS_MODBUS_CAB_VERSION_REG_NUMBER       2

#define BSS_MODBUS_BP_CONNECTED_STATE           0x40004
#define BSS_MODBUS_BP_STATUS                    0x40005
#define BSS_MODBUS_BP_STATE                     0x40006
#define BSS_MODBUS_BP_CYCLE                     0x40007
#define BSS_MODBUS_BP_SOC_SOH                   0x40008
#define BSS_MODBUS_BP_VOL                       0x40009
#define BSS_MODBUS_BP_CUR                       0x4000A

#define BSS_MODBUS_BP_CELL_0                    0x4000B
#define BSS_MODBUS_BP_CELL_1                    0x4000C
#define BSS_MODBUS_BP_CELL_2                    0x4000D
#define BSS_MODBUS_BP_CELL_3                    0x4000E
#define BSS_MODBUS_BP_CELL_4                    0x4000F
#define BSS_MODBUS_BP_CELL_5                    0x40010
#define BSS_MODBUS_BP_CELL_6                    0x40011
#define BSS_MODBUS_BP_CELL_7                    0x40012
#define BSS_MODBUS_BP_CELL_8                    0x40013
#define BSS_MODBUS_BP_CELL_9                    0x40014
#define BSS_MODBUS_BP_CELL_10                   0x40015
#define BSS_MODBUS_BP_CELL_11                   0x40016
#define BSS_MODBUS_BP_CELL_12                   0x40017
#define BSS_MODBUS_BP_CELL_13                   0x40018
#define BSS_MODBUS_BP_CELL_14                   0x40019
#define BSS_MODBUS_BP_CELL_15                   0x4001A

#define BSS_MODBUS_BP_ZONE_TEMP_01              0x4001B
#define BSS_MODBUS_BP_ZONE_TEMP_23              0x4001C
#define BSS_MODBUS_BP_ZONE_TEMP_45              0x4001D
#define BSS_MODBUS_BP_ZONE_TEMP_6               0x4001E

#define BSS_MODBUS_BP_SN                        0x4001F
#define BSS_MODBUS_BP_VERSION                   0x4002F

#define BSS_MODBUS_BP_UPGRADE_STATUS            0x4003F
#define BSS_MODBUS_BP_CANBUS_RX_FIFO_0          0x40040
#define BSS_MODBUS_BP_CANBUS_RX_FIFO_1          0x40045
#define BSS_MODBUS_BP_CANBUS_RX_FIFO_2          0x4004A
#define BP_CANBUS_RX_REG_SIZE                   5

enum {
    BSS_MODBUS_CAB_STATUS_TEMP_REG_INDEX = 0,
    BSS_MODBUS_CAB_POGO_TEMP_12_REG_INDEX,
    BSS_MODBUS_CAB_POGO_TEMP_34_REG_INDEX,

    BSS_MODBUS_BP_CONNECTED_STATE_REG_INDEX,
    BSS_MODBUS_BP_STATUS_REG_INDEX,
    BSS_MODBUS_BP_STATE_REG_INDEX,
    BSS_MODBUS_BP_CYCLE_REG_INDEX,
    BSS_MODBUS_BP_SOC_SOH_REG_INDEX,
    BSS_MODBUS_BP_VOL_REG_INDEX,
    BSS_MODBUS_BP_CUR_REG_INDEX,
    BSS_MODBUS_BP_CELL_0_REG_INDEX,
    BSS_MODBUS_BP_CELL_1_REG_INDEX,
    BSS_MODBUS_BP_CELL_2_REG_INDEX,
    BSS_MODBUS_BP_CELL_3_REG_INDEX,
    BSS_MODBUS_BP_CELL_4_REG_INDEX,
    BSS_MODBUS_BP_CELL_5_REG_INDEX,
    BSS_MODBUS_BP_CELL_6_REG_INDEX,
    BSS_MODBUS_BP_CELL_7_REG_INDEX,
    BSS_MODBUS_BP_CELL_8_REG_INDEX,
    BSS_MODBUS_BP_CELL_9_REG_INDEX,
    BSS_MODBUS_BP_CELL_10_REG_INDEX,
    BSS_MODBUS_BP_CELL_11_REG_INDEX,
    BSS_MODBUS_BP_CELL_12_REG_INDEX,
    BSS_MODBUS_BP_CELL_13_REG_INDEX,
    BSS_MODBUS_BP_CELL_14_REG_INDEX,
    BSS_MODBUS_BP_CELL_15_REG_INDEX,

    BSS_MODBUS_BP_ZONE_TEMP_01_REG_INDEX,
    BSS_MODBUS_BP_ZONE_TEMP_23_REG_INDEX,
    BSS_MODBUS_BP_ZONE_TEMP_45_REG_INDEX,
    BSS_MODBUS_BP_ZONE_TEMP_6_REG_INDEX,

    BSS_MODBUS_BP_SN_REG_INDEX,
    BSS_MODBUS_BP_VERSION_REG_INDEX,

    BSS_MODBUS_INPUT_REG_NUMBER
};

#define BSS_MODBUS_CAB_INPUT_REG_NUMBER     3
#define BSS_MODBUS_BP_INPUT_REG_NUMBER      (BSS_MODBUS_INPUT_REG_NUMBER - BSS_MODBUS_CAB_INPUT_REG_NUMBER)
#define BSS_MODBUS_SYNC_REG_NUMBER          30

/// BSS Modbus RTU Holding Register: 0x03
#define BSS_MODBUS_CAB_OPEN_DOOR                0x30003
#define BSS_MODBUS_CAB_CTL_FAN                  0x30002
#define BSS_MODBUS_CAB_CTL_CHARGER_SWITCH       0x30001
#define BSS_MODBUS_CAB_CTL_LED                  0x30004

#define BSS_MODBUS_CAB_CTL_STATE                0x30005
#define BSS_MODBUS_CAB_STATE_ACTIVE             0
#define BSS_MODBUS_CAB_STATE_BP_UPGRADING       1
#define BSS_MODBUS_CAB_STATE_REBOOT             2

#define BSS_MODBUS_BP_ASSIGNING                 0x30006
#define BSS_MODBUS_BP_REBOOT                    0x30007
#define BSS_MODBUS_BP_STATE_CONTROL             0x30008
#define BSS_MODBUS_BP_CHARGER_SWITCH            0x30009
#define BSS_MODBUS_BP_ASSIGNED_DEVICE           0x3000B

#define BSS_MODBUS_FILE_TYPE                    0x30020
#define BSS_MODBUS_FILE_TYPE_FW                 0
#define BSS_MODBUS_FILE_TYPE_RECORD             1

#define BSS_MODBUS_FILE_SIZE                    0x30021
#define BSS_MODBUS_FILE_CRC                     0x30022
#define BSS_MODBUS_FILE_ADDR_0                  0x30023
#define BSS_MODBUS_FILE_ADDR_1                  0x30024
#define BSS_MODBUS_FILE_TOTAL                   0x30025
#define BSS_MODBUS_FILE_ADDITIONAL              0x30026

#define BSS_MODBUS_FILE_ADDITIONAL_SIZE         2

#define BSS_MODBUS_FILE_CMD                     0x30028
#define BSS_MODBUS_FILE_CMD_COMPLETE            0
#define BSS_MODBUS_FILE_CMD_ABORT               1

#define BSS_MODBUS_FILE_FRAME_INDEX             0x30029
#define BSS_MODBUS_FILE_FRAME_DATA              0x30030
#define BSS_MODBUS_OTA_STATUS                   0x30075



#define BSS_MODBUS_FILE_MAX_LEN                 (64)

enum {
    BSS_MODBUS_CAB_OPEN_DOOR_REG_INDEX = 0,
    BSS_MODBUS_CAB_CTL_FAN_REG_INDEX,
    BSS_MODBUS_CAB_CTL_CHARGER_SWITCH_REG_INDEX,
    BSS_MODBUS_CAB_CTL_LED_REG_INDEX,
    BSS_MODBUS_CAB_REBOOT_REG_INDEX,

    BSS_MODBUS_BP_ASSIGNING_REG_INDEX,
    BSS_MODBUS_BP_REBOOT_REG_INDEX,
    BSS_MODBUS_BP_STATE_CONTROL_REG_INDEX,
    BSS_MODBUS_BP_CHARGER_SWITCH_REG_INDEX,
    BSS_MODBUS_BP_ASSIGNED_DEVICE_REG_INDEX,

    BSS_MODBUS_HOLDING_REG_NUMBER
};

#define BSS_MODBUS_TURN_ON                      (0x01)
#define BSS_MODBUS_TURN_OFF                     (0x00)
#define BSS_MODBUS_OPEN_DOOR                    BSS_MODBUS_TURN_ON

#endif //SM_MODBUS_DEFINE_H
