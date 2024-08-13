//
// Created by vuonglk on 09/05/2024.
//

#ifndef SM_QUEUE_H
#define SM_QUEUE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sm_queue {
    void *memory;
    size_t item_size;
    int32_t max_capacity;
    int32_t item_count;
} sm_queue_t;

sm_queue_t *sm_queue_create_default(int32_t _capacity, size_t _item_size);

sm_queue_t *sm_queue_create(int32_t _capacity, size_t _item_size);

int32_t sm_queue_destroy(sm_queue_t *_queue);

int32_t sm_queue_push(sm_queue_t *_queue, void *_item);

int32_t sm_queue_pop(sm_queue_t *_queue, void *_buff);

int32_t sm_queue_get_size(sm_queue_t *_queue);

void *sm_queue_get_font(sm_queue_t *_queue);

void *sm_queue_get_back(sm_queue_t *_queue);

#ifdef __cplusplus
};
#endif

#endif //SM_QUEUE_H
