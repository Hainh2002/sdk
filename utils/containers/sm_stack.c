/*
 * sm_stack.c
 *
 *  Created on: Jul 17, 2023
 *      Author: Admin
 */


#include "sm_stack.h"
#include <stdlib.h>
#include <string.h>

void sm_stack_init(sm_stack_t *p_stack,uint32_t length,size_t item_size){
    if(p_stack == NULL){
        p_stack = (sm_stack_t*) malloc(sizeof(sm_stack_t));
    }

    p_stack->length = length;
    p_stack->item_size = item_size;
    p_stack->item = (void*)calloc(length,item_size);
    p_stack->count = 0;

}
int32_t sm_stack_push(sm_stack_t *p_stack,void* item){
    if(p_stack == NULL) return -1;
    if((uint32_t)p_stack->count >= p_stack->length) return -1;
    memcpy(p_stack->item+(uint32_t)p_stack->count*p_stack->item_size,(void*)item,p_stack->item_size);
    p_stack->count++;
    return 0;
}
int32_t sm_stack_pop(sm_stack_t *p_stack,void* item){
    if(p_stack == NULL) return -1;
    if(p_stack->count <= 0) return -1;
    memcpy(item,p_stack->item + ((uint32_t)p_stack->count-1)*p_stack->item_size,p_stack->item_size);
    p_stack->count--;
    return 0;
}
int32_t sm_stack_deinit(sm_stack_t *p_stack){
    if(p_stack == NULL) return -1;
    free(p_stack->item);
    free(p_stack);
    return 0;
}
