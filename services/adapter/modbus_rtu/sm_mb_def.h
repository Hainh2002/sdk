//
// Created by vnbk on 01/12/2023.
//

#ifndef SM_MB_DEF_H
#define SM_MB_DEF_H

#define MODBUS_MAX_COILS_SUPPORT    		250
#define MODBUS_RESPONSE_TIMEOUT_DEFAULT 	200

typedef enum {
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
}MODBUS_FUNCTION_CODE;

typedef enum {
    // Library errors
    MODBUS_ERROR_INVALID_UNIT_ID = -7,  /**< Received invalid unit ID in response from server */
    MODBUS_ERROR_INVALID_TCP_MBAP = -6, /**< Received invalid TCP MBAP */
    MODBUS_ERROR_CRC = -5,              /**< Received invalid CRC */
    MODBUS_ERROR_TRANSPORT = -4,        /**< Transport error */
    MODBUS_ERROR_TIMEOUT = -3,          /**< Read/write timeout occurred */
    MODBUS_ERROR_INVALID_RESPONSE = -2, /**< Received invalid response from server */
    MODBUS_ERROR_INVALID_ARGUMENT = -1, /**< Invalid argument provided */
    MODBUS_ERROR_NONE = 0,              /**< No error */

    // Modbus exceptions
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 1,      /**< Modbus exception 1 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS = 2,  /**< Modbus exception 2 */
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE = 3,    /**< Modbus exception 3 */
    MODBUS_EXCEPTION_SERVER_DEVICE_FAILURE = 4, /**< Modbus exception 4 */
}MODBUS_ERROR_CODE;

#define MODBUS_GET_HIGH_BYTE_REG(value)         	(((value) >> 8) & 0xFF)
#define MODBUS_GET_LOW_BYTE_REG(value)          	((value) & 0xFF)

#define MODBUS_SET_HIGH_BYTE_REG(reg,value)         (((reg) & 0x00FF) |= ((value) << 8))
#define MODBUS_SET_LOW_BYTE_REG(reg,value)          ((reg) & 0xFF00 |= ((value) & 0xFF))

#endif //SM_MB_DEF_H
