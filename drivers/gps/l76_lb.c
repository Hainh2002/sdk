/*
 * gps.c
 *
 *  Created on: Dec 23, 2021
 *      Author: Ha
 */

#include "l76_lb.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
void l76_lb_split_nmea_message(char** fields, char* msg, char* delim_char){
    char** fields_buff = fields;
    char* msg_buff = msg;
    while( (*fields_buff++ = strtok_r(msg_buff, delim_char, &msg_buff)) );
}
bool covert_lat_lon_to_float(char* str, float* output){
	float value;
	if(strcmp(str,"") == 0) return false;
	value = (float)(atof(str));
	int hour;
	float minute;
	if(value < 100.0000) return false;
	hour = (int)(value / 100);
	minute = (float)((value - (float)hour*100) / 60);
	*output = (float)hour + minute;
	return true;
}

