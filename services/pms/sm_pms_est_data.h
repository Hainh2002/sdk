//
// Created by DELL 5425 on 4/10/2024.
//

#ifndef EV_SDK_SM_PMS_EST_DATA_H
#define EV_SDK_SM_PMS_EST_DATA_H

#include <stdint.h>
#include "sm_pms_data.h"
#include "sm_bp_data.h"
#include "sm_elapsed_timer.h"

#define PMS_EST_POWER_PER_KM_ECO        22
#define PMS_EST_POWER_PER_KM_SPORT      28

#define PMS_EST_1_BP_RESISTANCE         150
#define PMS_EST_2_BP_RESISTANCE         90
#define PMS_EST_3_BP_RESISTANCE         60

#define PMS_EST_DISTANCE_DURATION       500 // ms
#define PMS_EST_POWER_KM_DURATION       500 // ms
#define PMS_EST_CUR_MAX_DURATION        500 // ms

typedef struct est_time_cfg {
    int32_t m_dist_duration;
    int32_t m_power_duration;
    int32_t m_cur_duration;
}est_time_cfg_t;



typedef struct est_data {
    uint32_t            m_distance_km;
    uint32_t            m_distance_m;
    uint32_t            m_cur_max;
    uint32_t            m_power_per_km;
    elapsed_timer_t     m_dist_timeout;
    elapsed_timer_t     m_cur_timeout;
    elapsed_timer_t     m_pow_timeout;
    est_time_cfg_t      *m_duration;
}est_data_t;



/**
 * @brief est_data_cpy
 * @param _src
 * @param _des
 * @return
 */
static inline int32_t est_data_cpy(const est_data_t* _src, est_data_t* _des){
    if (_src == NULL) return 0;
    _des->m_distance_m      = _src->m_distance_m;
    _des->m_power_per_km    = _src->m_power_per_km;
    _des->m_cur_max         = _src->m_cur_max;
    _des->m_distance_km     = _src->m_distance_km;
    return 1;
}
/**
 * @brief sm_pms_reset_data
 * @param _est_data
 * @return
 */
static inline int32_t sm_pms_reset_data(est_data_t* _est_data){
    if (!_est_data) return -1;

    _est_data->m_distance_km        = 0;
    _est_data->m_distance_m         = 0;
    _est_data->m_power_per_km       = 0;
    _est_data->m_cur_max            = 0;

    elapsed_timer_resetz(&_est_data->m_dist_timeout, _est_data->m_duration->m_dist_duration);
    elapsed_timer_resetz(&_est_data->m_cur_timeout, _est_data->m_duration->m_cur_duration);
    elapsed_timer_resetz(&_est_data->m_pow_timeout, _est_data->m_duration->m_power_duration);

    return 0;
}

int32_t sm_pms_est_process(est_data_t* _est_data, pms_port_t * _pms_port);

#endif //EV_SDK_SM_PMS_EST_DATA_H
