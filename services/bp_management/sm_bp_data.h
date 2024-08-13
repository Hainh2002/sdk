#ifndef SM_BP_DATA_H
#define SM_BP_DATA_H

#include <stdint.h>
#include <string.h>
#include "sm_types.h"

#define BP_CELL_TEMP_SIZE               6
#define BP_CELL_VOL_SIZE                16
#define BP_VERSION_SIZE                 32
#define BP_DEVICE_SN_SIZE               32

#define BP_VOL_MAX                      67200 //mV
#define BP_VOL_MIN                      48000 //mV

enum {
    BP_STATE_INIT = 0,
    BP_STATE_IDLE,
    BP_STATE_SOFT_START,
    BP_STATE_DISCHARGING,
    BP_STATE_CHARGING,
    BP_STATE_FAULT,
    BP_STATE_SHIP_MODE,
    BP_STATE_SYSTEM_BOOST_UP,
    BP_STATE_ID_ASSIGN_START,
    BP_STATE_ID_ASSIGN_WAIT_CONFIRM,
    BP_STATE_ID_ASSIGN_CONFIRMED,
    BP_STATE_ID_ASSIGN_WAIT_SLAVE_SELECT,
    BP_STATE_START_AUTHENTICATE,
    BP_STATE_AUTHENTICATING,
    BP_STATE_STANDBY,
    BP_STATE_SHUTDOWN,
    BP_STATE_ONLY_DISCHARGING
};

typedef struct sm_bp_data{
    char m_sn[BP_DEVICE_SN_SIZE];
    int32_t m_vol;
    int32_t m_cur;
    int32_t m_state;
    int32_t m_status;
    int32_t m_soc;
    int32_t m_soh;
    int32_t m_cycle;
    uint8_t m_temps[BP_CELL_TEMP_SIZE];
    uint16_t m_cellVols[BP_CELL_VOL_SIZE];

    char m_version[BP_VERSION_SIZE];
    char m_assignedSn[BP_DEVICE_SN_SIZE];
}sm_bp_data_t;

static inline void sm_bp_reset_data(sm_bp_data_t* _this){
    memset(_this->m_sn, '\0', BP_DEVICE_SN_SIZE);
    _this->m_vol = 0;
    _this->m_cur = 0;
    _this->m_state = 0;
    _this->m_status = 0;
    _this->m_soc = -1;
    _this->m_soh = -1;
    _this->m_cycle = -1;

    memset(_this->m_temps, 0, BP_CELL_TEMP_SIZE);
    memset(_this->m_cellVols, 0, BP_CELL_VOL_SIZE);

    memset(_this->m_version, '\0', BP_VERSION_SIZE);
    memset(_this->m_assignedSn, '\0', BP_DEVICE_SN_SIZE);
}

static inline void sm_bp_clone_data(sm_bp_data_t* _this, const sm_bp_data_t* _other){
    if(!_this || !_other){
        return;
    }
    memcpy(_this->m_sn, _other->m_sn, BP_DEVICE_SN_SIZE);

    _this->m_vol = _other->m_vol;
    _this->m_cur = _other->m_cur;
    _this->m_state = _other->m_state;
    _this->m_status = _other->m_status;
    _this->m_soc = _other->m_soc;
    _this->m_soh = _other->m_soh;
    _this->m_cycle = _other->m_cycle;

    memcpy(_this->m_temps, _other->m_temps, BP_CELL_TEMP_SIZE);
    memcpy(_this->m_cellVols, _other->m_cellVols, BP_CELL_VOL_SIZE);

    memcpy(_this->m_version, _other->m_version, BP_VERSION_SIZE);
    memcpy(_this->m_assignedSn, _other->m_assignedSn, BP_DEVICE_SN_SIZE);   
}

typedef enum{
    BP_CMD_REBOOT ,
    BP_CMD_CHARGE,
    BP_CMD_ONLY_DISCHARGE,
    BP_CMD_DISCHARGE,
    BP_CMD_STANDBY,
    BP_CMD_READ_SN,
    BP_CMD_READ_ASSIGNED_DEV,
    BP_CMD_WRITE_ASSIGNED_DEV,
    BP_CMD_READ_VERSION,
    BP_CMD_NUMBER
}SM_BP_CMD;

#define SM_BP_CMD_SUCCESS    (0)
#define SM_BP_CMD_FAILURE    (-1)

/**
 * @brief
 * @param id:
 * @param cmd
 * @param is_success
 * @param data
 * @param arg
*/
typedef void (*sm_bp_on_cmd_fn_t)(int32_t, SM_BP_CMD, int32_t, void*, void*);

typedef enum{
    BP_AUTH_FAILURE,
    BP_AUTH_SUCCESS,
    BP_AUTH_EVENT_NUMBER
}SM_BP_AUTH_EVENT;

typedef void (*sm_bp_auth_event_fn_t)(int32_t, SM_BP_AUTH_EVENT, const char*, int32_t, void*);


#endif