/*
 * sm_map.h
 *
 *  Created on: Sep 6, 2023
 *      Author: Admin
 */

#ifndef UTILS_INCLUDE_SM_MAP_H_
#define UTILS_INCLUDE_SM_MAP_H_

#include "stdint.h"
#include "string.h"
#include "stdio.h"

typedef struct sm_map sm_map_t;

struct sm_map{
	char** key;
	void **value;
	size_t size;
	size_t count;
	uint32_t index;
};
/**
 * @brief
 * @param _this
 * @param _count
 * @param size
 */
void sm_map_init(sm_map_t *_this,size_t _count,size_t size);
/**
 * @brief
 * @param _this
 * @param _key
 * @param _value
 */
void sm_map_insert(sm_map_t *_this,char* _key,void *_value);
/**
 * @brief
 * @param _this
 * @param _key
 */
void* sm_map_search(sm_map_t *_this,char* _key);
/**
 * @brief
 * @param _this
 */
void sm_map_deinit(sm_map_t *_this);


#endif /* UTILS_INCLUDE_SM_MAP_H_ */
