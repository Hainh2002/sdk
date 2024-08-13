/*
 * sm_lowpass_filter.c
 *
 *  Created on: Sep 7, 2023
 *      Author: Admin
 */


#include "sm_lowpass_filter.h"

#include "math.h"
#define  M_PI 3.14
/**
 *
 * @param _this
 * @param _cutOffFrequency
 * @param _freq_sample_rate
 */
void sm_lowpass_init(sm_lowpass_f_t *_this, float _cutOffFrequency, float _freq_sample_rate){
	_this->output = 0;
	_this->epow = 1-exp(-(1/_freq_sample_rate) * 2 * M_PI * _cutOffFrequency);
}
/**
 *
 * @param _this
 * @param _input
 * @return
 */
float sm_lowpass_update(sm_lowpass_f_t *_this,float _input){
	_this->output += (_input - _this->output) * _this->epow;
	return sm_lowpass_get_update(_this);
}
