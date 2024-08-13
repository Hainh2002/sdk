/*
 *
 *
 *  Created on: Jul 22, 2022
 *      Author: Minh Nhan
 */

#ifndef FIFO_H_
#define FIFO_H_


#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"
#include "string.h"


#define NO_RTOS 1
#if NO_RTOS
#else
#include "FreeRTOS.h"
#include "queue.h"
#endif

typedef struct sm_fifo_handle sm_fifo_handle_t;

struct sm_fifo_handle{
#if NO_RTOS
    void *buff;
    size_t length;
    size_t size;
    volatile int head;
    volatile int tail;
    volatile unsigned int count;
    volatile bool lock;
#else
    StaticQueue_t static_queue;
    QueueHandle_t queue;
    size_t length;
    size_t size;

#endif


};

/**
 * @brief
 * @param p_fifo
 * @param length
 * @param size
 */
void sm_fifo_init(sm_fifo_handle_t *p_fifo,size_t length,size_t size);
/**
 * @brief
 * @param length
 * @param size
 * @return
 */
sm_fifo_handle_t *sm_fifo_new_(size_t length,size_t size);
/**
 * @brief
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_push(sm_fifo_handle_t *p_fifo,void *item);
/**
 * @brief
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_pop(sm_fifo_handle_t *p_fifo,void *item);
/**
 * @brief
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_push_irq(sm_fifo_handle_t *p_fifo, void *item);
/**
 * @brief
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_pop_irq(sm_fifo_handle_t *p_fifo, void *item);
/**
 * @brief
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_peek(sm_fifo_handle_t* p_fifo, void* item);
/**
 * @brief
 * @param p_fifo
 */
void sm_fifo_deinit(sm_fifo_handle_t *p_fifo);
/**
 * @brief
 * @param p_fifo
 * @return
 */
unsigned int sm_fifo_get_num_item(sm_fifo_handle_t *p_fifo);

#endif
