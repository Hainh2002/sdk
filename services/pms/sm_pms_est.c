//
// Created by DELL 5425 on 4/15/2024.
//
#include "sm_pms_est_data.h"
int32_t ocv_table[101] = {
        53000,	53000,	53000,	53000,	53000,	53000,	55114,	55203,	55291,	55371,
        55440,	55520,	55619,	55763,	55962,	56146,	56512,	56656,	56774,	56890,
        57003,	57112,	57206,	57301,	57381,	57445,	57525,	57589,	57658,	57723,
        57778,	57827,	57877,	57915,	57960,	58005,	58050,	58090,	58139,	58179,
        58224,	58278,	58323,	58373,	58422,	58472,	58531,	58590,	58650,	58715,
        58784,	58858,	58942,	59037,	59155,	59299,	59469,	59662,	59845,	60010,
        60142,	60272,	60402,	60530,	60659,	60794,	60931,	61070,	61210,	61349,
        61493,	61642,	61790,	62093,	62246,	62405,	62563,	62723,	62882,	63050,
        63213,	63378,	63546,	63715,	63883,	64067,	64245,	64429,	64608,	64800,
        64984,	65173,	65366,	65565,	65763,	65971,	66179,	66394,	66621,	66854,	67117
};

static uint8_t is_bp_over_heat = 0;

static void sm_pms_max_cur_cal(est_data_t* _est_data, pms_port_t* _pms_port){
    int32_t bps_res = 1;
    int32_t cur_max = 0;
    int32_t bp_discharging_num = 0;
    int32_t bps_soc = 0;
    int32_t bps_soc_old = 0;
    int32_t ocv = 0;

    for (int i = 0; i < SM_SV_PMS_MAX_BP_NUM_DEFAULT; ++i) {
        if (_pms_port[i].m_bp_data->m_state == BP_STATE_DISCHARGING) {
            bp_discharging_num++;
            bps_soc_old = bps_soc;
            bps_soc += _pms_port[i].m_bp_data->m_soc;
            if (bps_soc < 1)
                bps_soc = 1;
            if ((bps_soc > bps_soc_old) && (bps_soc_old != 0))
                bps_soc = bps_soc_old;
        }
    }
    if (bps_soc < 1) bps_soc = 1;
    if (bps_soc > 99) bps_soc = 99;

    ocv = ocv_table[bps_soc];

    if (bp_discharging_num == 1) bps_res = PMS_EST_1_BP_RESISTANCE;
    else if (bp_discharging_num == 2) bps_res = PMS_EST_2_BP_RESISTANCE;
    else if (bp_discharging_num == 3) bps_res = PMS_EST_3_BP_RESISTANCE;

    cur_max = 1000*(ocv-53000)/bps_res;

    switch (bp_discharging_num) {
        case 1:
            if ( bps_soc <= 20 ) {cur_max = cur_max - 2000;}
            if ( cur_max >= 35000) 	{cur_max = 35000;}
            if ( cur_max <= 5000) 	{cur_max = 5000 ;}
            /* active overheat cur limit level 1*/
//            if((_pms_port->m_port->m_bp_data->m_temps[0] > 55) && (cur_max > 20000)){
//                cur_max = 20000 ;
//                is_bp_over_heat = 1;
//            }else{
//                is_bp_over_heat = 0;
//            }
            break;
        case 2:
            if ( bps_soc <= 20 ) {cur_max = cur_max - 2000;}
            if ( cur_max >= 45000) 	{cur_max = 45000;}
            if ( cur_max <= 1000) 	{cur_max = 10000;}
            break;
        case 3:
            if ( bps_soc <= 20 ) {cur_max = cur_max - 2000;}
            if ( cur_max >= 50000) 	{cur_max = 50000;}
            if ( cur_max <= 15000) 	{cur_max = 15000;}
            break;
        default:
            break;
    }
    _est_data->m_cur_max = cur_max;
}

static void sm_pms_power_per_km_cal(est_data_t* _est_data){
    /* */
}

static void sm_pms_est_distance(est_data_t* _est_data, pms_port_t* _pms_port){
    float power_unused;
    uint8_t drv_mode = 0;
    float power_km_f = PMS_EST_POWER_PER_KM_ECO;
    int8_t bps_num = SM_SV_PMS_MAX_BP_NUM_DEFAULT;
    uint32_t soc_total_unused = 0;
    uint32_t distance_km = 0;

#if SM_SV_PMS_MOTOR_CONTROLLER_CONNECTION
    uint8_t mc_drv_mode = _pms_port->m_mc.drv_mode;
    if(mc_drv_mode == 0x00 || mc_drv_mode == 0x01 || mc_drv_mode == 0x02) {
        drv_mode = 0;
    }
    else {
        drv_mode = 1 ;
    }

    if( drv_mode == 0 ){
        power_km_f = PMS_EST_POWER_PER_KM_ECO;
    }
    else{
        power_km_f = PMS_EST_POWER_PER_KM_SPORT;
    }
#endif
    for ( uint8_t i= 0; i < bps_num; i++){
        if( _pms_port[i].m_connection_st == PORT_ST_ENABLE &&
            _pms_port[i].m_valid_st != PORT_ST_DISABLE)
        {
            soc_total_unused += _pms_port[i].m_bp_data->m_soc*10;
        }
    }

    if ( power_km_f < 1)	power_km_f =1;
    if ( soc_total_unused < 50 ){// soc x10
        distance_km = 0;
    }
    else/* estimate distance = %SoC*sum_power/power_per_km )(km) */
    {
        power_unused = (float) (soc_total_unused - 50)*11;// 10 is the power of BP divided by 100
        distance_km = (uint32_t) ((power_unused/power_km_f)/10);// power_unused x 10
    }
    if ( distance_km <= 0 ) distance_km = 0;
    if ( distance_km >= 250) distance_km = 250;

    _est_data->m_distance_km = distance_km;
}


int32_t sm_pms_est_process(est_data_t* _est_data, pms_port_t* _pms_port){
    if (!_pms_port) return -1;
    if(!elapsed_timer_get_remain(&_est_data->m_dist_timeout)){
        sm_pms_est_distance(_est_data, _pms_port);
        elapsed_timer_reset(&_est_data->m_dist_timeout);
    }
    if(!elapsed_timer_get_remain(&_est_data->m_cur_timeout)){
        sm_pms_max_cur_cal(_est_data, _pms_port);
        elapsed_timer_reset(&_est_data->m_cur_timeout);
    }
    if(!elapsed_timer_get_remain(&_est_data->m_pow_timeout)){
        sm_pms_power_per_km_cal(_est_data);
        elapsed_timer_reset(&_est_data->m_pow_timeout);
    }
    return 0;
}