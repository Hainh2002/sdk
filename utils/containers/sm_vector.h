//
// Created by vuonglk on 06/05/2024.
//

#ifndef SM_VECTOR_H
#define SM_VECTOR_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct sm_vector
{
    void* memory;
    size_t item_size;
    uint32_t max_capacity;
    uint32_t item_count;
} sm_vector_t;

sm_vector_t* sm_vector_create(uint32_t _capacity, size_t _item_size);

int32_t sm_vector_push_back(sm_vector_t* _vector, void* _item);

int32_t sm_vector_push_font(sm_vector_t* _vector, void* _item);

int32_t sm_vector_copy_item(sm_vector_t* _vector, uint32_t _index, void* _buff);

void* sm_vector_get_item(sm_vector_t* _vector, uint32_t _index);

void* sm_vector_get_font(sm_vector_t* _vector);

void* sm_vector_get_back(sm_vector_t* _vector);

int32_t sm_vector_get_size(sm_vector_t* _vector);

bool sm_vector_is_full(sm_vector_t* _vector);

int32_t sm_vector_copy_front(sm_vector_t* _vector, void* _buff);

int32_t sm_vector_copy_last(sm_vector_t* _vector, void* _buff);

int32_t sm_vector_erase_item_by_index(sm_vector_t* _vector, uint32_t _index);

int32_t sm_vector_erase_item(sm_vector_t* _vector, void* _item);

int32_t sm_vector_destroy(sm_vector_t* _vector);


#endif //SM_VECTOR_H
