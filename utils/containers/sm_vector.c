//
// Created by vuonglk on 06/05/2024.
//
#include "sm_vector.h"
#include <string.h>

sm_vector_t* sm_vector_create(uint32_t _capacity, size_t _item_size)
{
    sm_vector_t* vector = malloc(sizeof(sm_vector_t));
    if (vector == NULL)
    {
        printf("cant create vector instance");
        return NULL;
    }
    vector->max_capacity = _capacity;
    vector->item_size = _item_size;
    vector->memory = malloc(_capacity * _item_size);
    if (vector->memory == NULL)
    {
        free(vector);
        printf("cant create vector memory");
        return NULL;
    }
    vector->item_count = 0;
    return vector;
}

int32_t sm_vector_push_back(sm_vector_t* _vector, void* _item)
{
    if (_vector == NULL) return -1;
    if (_vector->item_count >= _vector->max_capacity) return -1;

    memcpy(_vector->memory + (_vector->item_count * _vector->item_size), _item, _vector->item_size);
    _vector->item_count++;
    return 0;
}

int32_t sm_vector_push_font(sm_vector_t* _vector, void* _item){
    if (_vector == NULL) return -1;
    if (_vector->item_count >= _vector->max_capacity) return -1;

    memmove(_vector->memory  + _vector->item_size, _vector->memory, _vector->item_count * _vector->item_size);
    memcpy(_vector->memory, _item, _vector->item_size);
    _vector->item_count++;
    return 0;
}

int32_t sm_vector_get_size(sm_vector_t* _vector)
{
    if (_vector == NULL) return -1;
    return _vector->item_count;
}

bool sm_vector_is_full(sm_vector_t* _vector)
{
    if (_vector == NULL) return true;
    return _vector->item_count >= _vector->max_capacity;
}

int32_t sm_vector_copy_item(sm_vector_t* _vector, uint32_t _index, void* _buff)
{
    if (_vector == NULL) return -1;
    if(_vector->item_count == 0) return -1;
    if (_index >= _vector->item_count) return -1;

    memcpy(_buff, _vector->memory + (_index * _vector->item_size), _vector->item_size);
    return 0;
}

void* sm_vector_get_item(sm_vector_t* _vector, uint32_t _index)
{
    if (_vector == NULL) return NULL;
    if(_vector->item_count == 0) return NULL;
    if (_index >= _vector->item_count) return NULL;
    unsigned char	*ptr;
    ptr = _vector->memory + _vector->item_size*_index;
    return ptr;
}


void* sm_vector_get_font(sm_vector_t* _vector){
    if (_vector == NULL) return NULL;
    if(_vector->item_count == 0) return NULL;
    unsigned char	*ptr;
    ptr = _vector->memory;
    return ptr;
}

void* sm_vector_get_back(sm_vector_t* _vector){
    if (_vector == NULL) return NULL;
    if(_vector->item_count == 0) return NULL;
    unsigned char	*ptr;
    ptr = _vector->memory + (_vector->item_count - 1)*_vector->item_size;
    return ptr;
}


int32_t sm_vector_copy_front(sm_vector_t* _vector, void* _buff)
{
    if (_vector == NULL) return -1;
    if (_buff == NULL) return -1;
    if(_vector->item_count == 0) return -1;

    memcpy(_buff, _vector->memory, _vector->item_size);
    return 0;
}

int32_t sm_vector_copy_last(sm_vector_t* _vector, void* _buff)
{
    if (_vector == NULL) return -1;
    if (_buff == NULL) return -1;
    if(_vector->item_count == 0) return -1;

    memcpy(_buff, _vector->memory + ((_vector->item_count - 1) * _vector->item_size), _vector->item_size);
    return 0;
}

int32_t sm_vector_erase_item_by_index(sm_vector_t* _vector, uint32_t _index)
{
    if (_vector == NULL) return -1;
    if (_index >= _vector->item_count) return -1;
    if(_vector->item_count == 0) return -1;

    if (_index == (_vector->item_count - 1))
    {
        _vector->item_count--;
    }
    else
    {
        void* erase_pos = _vector->memory + (_index * _vector->item_size);
        memmove(erase_pos, erase_pos + _vector->item_size, (_vector->item_count - _index - 1) * _vector->item_size);
        _vector->item_count--;
    }
    return 0;
}

int32_t sm_vector_erase_item(sm_vector_t* _vector, void* _item)
{
    if (_vector == NULL) return -1;
    if (_item == NULL) return -1;
    void* begin = _vector->memory;
    void* end = _vector->memory + _vector->item_size*(_vector->item_count - 1);
    if(_item < begin || _item > end)
        return -1;

    if (_item == _vector->memory + _vector->item_size*(_vector->item_count - 1))
    {
        _vector->item_count--;
    }
    else
    {
        memmove(_item, _item + _vector->item_size, end - _item);
        _vector->item_count--;
    }

    return 0;
}

int32_t sm_vector_destroy(sm_vector_t* _vector)
{
    if (_vector == NULL) return -1;
    free(_vector->memory);
    free(_vector);
    return 0;
}
