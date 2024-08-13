/*
 * list.h
 *
 *  Created on: Jul 14, 2023
 *      Author: Admin
 */

#ifndef UTILS_LIST_H_
#define UTILS_LIST_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct link_list link_list_t;

struct link_list{
  void *item;
  size_t size_item;
  link_list_t *next;
};
/**
 * @brief
 * @param item
 * @param size_item
 * @return
 */
link_list_t * sm_list_new(void* item,size_t size_item);
/**
 * @brief
 * @param list
 */
void sm_list_delete(link_list_t *list);
/**
 * @brief
 * @param list
 * @param item
 * @return
 */
int32_t sm_list_add(link_list_t *list,void* item);
/**
 * @brief
 * @param list
 * @param item
 * @return
 */
int32_t sm_list_remove(link_list_t *list,void *item);
/**
 * @brief
 * @param list
 */
void sm_list_sort(link_list_t *list);

#endif /* UTILS_LIST_H_ */
