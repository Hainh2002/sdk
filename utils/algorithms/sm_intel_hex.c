/*
 * intel_hex.c
 *
 *  Created on: Mar 14, 2023
 *      Author: Admin
 */

#include <stdint.h>
#include <stdio.h>

#include "sm_intel_hex.h"

static const char* TAG = "INTEL_HEX";

intel_hex hex_record;

uint8_t hex_data[32];

intel_hex* intel_hex_process(uint8_t* data, uint16_t length){
	intel_hex* result = &hex_record;
	if(data[0] == ':'){
		result->start_code = data[0];
		result->byte_count = data[1];
		result->addr[0] = data[2];
		result->addr[1] = data[3];
		result->record_type = data[4];
		result->data = hex_data;
		for(uint16_t i = 0;i<result->byte_count;i++){
			result->data[i] = data[5+i];
		}
		result->checksum = data[5+result->byte_count];
		if(result->checksum != intel_hex_checksum(data, length)){
			printf("Checksum error\n");
			return NULL;
		}
	}
	else{
		printf("Not intel hex data\n");
		return NULL;
	}
	return result;
}

uint8_t intel_hex_checksum(uint8_t* data, uint16_t length){
	uint8_t intel_crc = 0;
	for(uint16_t i = 1;i<length-1;i++){
		intel_crc = intel_crc + data[i];
	}
	intel_crc = (~intel_crc + 1);
	return intel_crc;
}

uint8_t hex[22] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
		'B', 'C', 'D', 'E', 'F', 'a', 'b', 'c', 'd', 'e', 'f' };
static uint8_t byte_to_hex(uint8_t byte);
static uint8_t byte_to_hex(uint8_t byte) {
	uint8_t _hex = 0xff;
	for (uint16_t i = 0; i < 22; i++) {
		if (byte == hex[i]) {
			if (i < 10) {
				_hex = byte - 48;
			} else if (i >= 10 && i < 16) {
				_hex = byte - 55;
			} else {
				_hex = byte - 87;
			}
		}
	}

	if(_hex == 0xff) _hex = byte;

	return _hex;
}

int string_to_hex(char* data, uint16_t length,uint8_t *dest){
	uint8_t *result = dest;
	uint16_t p_result = 0;
	for (uint16_t i = 0; i < length; i = i + 2) {
		uint8_t high_byte = byte_to_hex((uint8_t) data[i]);
		uint8_t low_byte = byte_to_hex((uint8_t) data[i + 1]);
		if (high_byte == data[i]) {
			result[p_result] = high_byte;
			p_result++;
			i -= 1;
			continue;
		}
		result[p_result] = (uint8_t) (high_byte << 4 | low_byte);
		p_result++;
	}
	return p_result;
}

uint16_t byte_to_half_word(uint8_t* src){
	return (uint16_t)((uint16_t)src[0]<<8 | (uint16_t)src[1]);
}

uint32_t byte_to_word(uint8_t* src){
	uint32_t result = 0;
	for(uint16_t i = 0;i<4;i++){
		result = result | (uint32_t)src[i]<< 8*(3-i);
	}
	return result;
}
uint32_t byte_to_word_cr(uint8_t* src){
	uint32_t result = 0;
	for(uint16_t i = 0;i<4;i++){
		result = result | (uint32_t)src[i]<< 8*(i);
	}
	return result;
}
