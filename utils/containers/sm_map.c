/*
 * sm_map.c
 *
 *  Created on: Sep 6, 2023
 *      Author: Admin
 */


#include "sm_map.h"
#include "stdlib.h"
/**
 *
 * @param _this
 * @param _count
 * @param _size
 */
void sm_map_init(sm_map_t *_this,size_t _count,size_t _size){
	_this->size = _size;
	_this->count = _count;
	_this->key = malloc(_this->count*sizeof(char*));
	_this->value = malloc(_this->count*sizeof(void*));
	for(uint16_t i = 0;i<_this->count;i++){
		_this->key[i] = malloc(20*sizeof(char));
		_this->value[i] = malloc(_size);
		memset(_this->key[i],0,20);
	}

	_this->index = 0;
}
/**
 *
 * @param _this
 * @param _key
 * @param _value
 */
void sm_map_insert(sm_map_t *_this,char* _key,void* _value){
	void *p_value = sm_map_search(_this, _key);

	if(p_value == NULL){
		if(_this->index < _this->count){
			memcpy(_this->key[_this->index], _key,strlen(_key));
			memcpy(_this->value[_this->index], _value,sizeof(_this->size));
			_this->index++;
			return;
		}
		return;
	}
	memcpy(p_value,_value,sizeof(_this->size));
}
/**
 *
 * @param _this
 * @param _key
 */
void* sm_map_search(sm_map_t *_this,char* _key){
	for(uint16_t i = 0;i<_this->count;i++){
		if(strcmp(_this->key[i],_key) == 0){
			return (void*)_this->value[i];
		}
	}
	return NULL;
}
/**
 *
 * @param _this
 */
void sm_map_deinit(sm_map_t *_this){
	for(uint16_t i = 0;i<_this->count;i++){
		free(_this->key[i]);
		free(_this->value[i]);
	}
	free(_this->key);
	free(_this->value);
}
