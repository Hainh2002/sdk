/*
 * cfifo.c
 *
 *  Created on: Jul 22, 2022
 *      Author: Minh Nhan
 */
#include <sm_fifo.h>
#include <assert.h>

#if NO_RTOS

bool sm_fifo_push_irq(sm_fifo_handle_t *p_fifo, void *item){
        p_fifo->lock = true;
        unsigned int next = (unsigned int) (p_fifo->head + 1) % p_fifo->length;
        if (next != p_fifo->tail) {
    //        p_fifo->m_buff[p_fifo->m_head] = c;
            memcpy((p_fifo->buff+p_fifo->head*p_fifo->size),item,p_fifo->size);
            p_fifo->head = next;
            p_fifo->count += 1;
            p_fifo->lock = false;
            return true;
        }
        p_fifo->lock = false;
        //printf("fifo push false\r\n");
        return false;
}
bool sm_fifo_pop_irq(sm_fifo_handle_t *p_fifo, void *item){
        p_fifo->lock = true;

        if (p_fifo->head != p_fifo->tail) {
            // data[0] = p_fifo->m_buff[p_fifo->m_tail];
            memcpy(item,(p_fifo->buff+p_fifo->tail*p_fifo->size),p_fifo->size);
            p_fifo->tail = (unsigned int) (p_fifo->tail + 1) % p_fifo->length;
            p_fifo->count -= 1;
            p_fifo->lock = false;
            return true;
        }
        p_fifo->lock = false;
        //printf("fifo pop false\r\n");
        return false;
}

void sm_fifo_init(sm_fifo_handle_t *p_fifo,size_t length,size_t size){
    if(p_fifo == NULL){
        p_fifo = (sm_fifo_handle_t*)malloc(sizeof(sm_fifo_handle_t));
    }
    assert(p_fifo != NULL);
    p_fifo->length = length+1;
    p_fifo->size = size;
    p_fifo->count = 0;
    p_fifo->head = -1;
    p_fifo->tail = -1;
    p_fifo->buff = calloc(p_fifo->length,p_fifo->size);
    p_fifo->lock = false;
    if(p_fifo->buff == NULL){
    	printf("fifo init false\r\n");
    	return;
    }
}
void sm_fifo_deinit(sm_fifo_handle_t *p_fifo){
	p_fifo->count = 0;
	p_fifo->head = 0;
	p_fifo->tail = 0;
	p_fifo->length = 0;
	free(p_fifo->buff);
	if(p_fifo->buff != NULL){
		//printf("deinit false\r\n");
	}
}
bool sm_fifo_push(sm_fifo_handle_t *p_fifo,void *item){
    p_fifo->lock = true;
    unsigned int next = (unsigned int) (p_fifo->head + 1) % p_fifo->length;
    if (next != p_fifo->tail) {
//        p_fifo->m_buff[p_fifo->m_head] = c;
        memcpy((p_fifo->buff+p_fifo->head*p_fifo->size),item,p_fifo->size);
        p_fifo->head = next;
        p_fifo->count += 1;
        p_fifo->lock = false;
        return true;
    }
    p_fifo->lock = false;
    //printf("fifo push false\r\n");
    return false;
}
bool sm_fifo_pop(sm_fifo_handle_t *p_fifo,void *item){
    p_fifo->lock = true;

	if (p_fifo->head != p_fifo->tail) {
		// data[0] = p_fifo->m_buff[p_fifo->m_tail];
        memcpy(item,(p_fifo->buff+p_fifo->tail*p_fifo->size),p_fifo->size);
		p_fifo->tail = (unsigned int) (p_fifo->tail + 1) % p_fifo->length;
		p_fifo->count -= 1;
        p_fifo->lock = false;
		return true;
	}
    p_fifo->lock = false;
	//printf("fifo pop false\r\n");
	return false;
}
unsigned int sm_fifo_get_num_item(sm_fifo_handle_t *p_fifo){
	return p_fifo->count;
}

bool sm_fifo_peek(sm_fifo_handle_t* p_fifo, void* item){
	return false;
}

#else
#include "FreeRTOS.h"
#include "queue.h"

sm_fifo_handle_t *sm_fifo_new_(size_t length,size_t size){
	sm_fifo_handle_t *_this = NULL;

	sm_fifo_init(_this,length,size);

	return _this;

}
/**
 *
 * @param p_fifo
 * @param length
 * @param size
 */
void sm_fifo_init(sm_fifo_handle_t *p_fifo, size_t length, size_t size)
{
    assert(p_fifo != NULL);

    uint8_t *area = malloc (length * size);

    p_fifo->queue = xQueueCreateStatic(length, size, area, &p_fifo->static_queue);
}
/**
 *
 * @param p_fifo
 */
void sm_fifo_deinit(sm_fifo_handle_t *p_fifo)
{
    (void) p_fifo;
}
/**
 *
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_push(sm_fifo_handle_t *p_fifo, void *item)
{
    xQueueSend(p_fifo->queue, item, 10);
    return true;
}
/**
 *
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_pop(sm_fifo_handle_t *p_fifo, void *item)
{
    if (xQueueReceive (p_fifo->queue, item, 1) == pdPASS)
    {
        return true;
    }
    return false;
}
/**
 *
 * @param p_fifo
 * @return
 */
unsigned int sm_fifo_get_num_item(sm_fifo_handle_t *p_fifo)
{
    return 1;
}
/**
 *
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_push_irq(sm_fifo_handle_t *p_fifo, void *item)
{
    BaseType_t mask;
    xQueueSendFromISR(p_fifo->queue, item, &mask);
    return true;
}
/**
 *
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_pop_irq(sm_fifo_handle_t *p_fifo, void *item)
{
    BaseType_t mask;
    xQueueReceiveFromISR (p_fifo->queue, item, &mask);
    return true;
}
/**
 *
 * @param p_fifo
 * @param item
 * @return
 */
bool sm_fifo_peek(sm_fifo_handle_t* p_fifo, void* item){
	if(xQueuePeek(p_fifo->queue, item, 1) == pdPASS)
	{
		return true;
	}
	return false;
}
#endif
