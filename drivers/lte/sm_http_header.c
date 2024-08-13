/*
 * sm_http_header.c
 *
 *  Created on: Jul 25, 2023
 *      Author: Admin
 */


#include "sm_http_header.h"
#include <stdlib.h>
#include <assert.h>

sm_http_header_t* sm_http_header_new(char *key, char *value) {
	if (key == NULL || value == NULL)
		return NULL;
	sm_http_header_t *node = (sm_http_header_t*) malloc(
			sizeof(sm_http_header_t));

	node->key = key;
	node->value = value;
	node->next = NULL;
	return node;

}
sm_http_header_t* sm_http_header_delete(sm_http_header_t *list)
{
	if (list == NULL)
        return NULL;
	sm_http_header_t *node = list->next;
	list->key = NULL;
	list->value = NULL;
	free(list);
	list = NULL;
    assert(list == NULL);
    return sm_http_header_delete (node);
}
int32_t sm_http_header_add(sm_http_header_t *list, char *key, char *value) {
	if (list->next == NULL) {
		list->next = sm_http_header_new(key, value);
		if (list->next == NULL)
			return -1;
		return 0;
	}
	return sm_http_header_add(list->next, key, value);
}
int32_t sm_http_header_remove(sm_http_header_t *list, char *key) {
	sm_http_header_t *node = list;
	if (node == NULL)
		return -1;
	
	if (strcmp(node->key, key) == 0) {
		sm_http_header_t *node_find = node;
		node = node->next;
		free(node_find);
		return 0;
	}
	node = node->next;
	return sm_http_header_remove(node, key);
}
sm_http_header_t* sm_http_header_find(sm_http_header_t *list, char *key)
{
	if (list == NULL)
		return NULL;
	if (strcmp(list->key, key) == 0)
        return list;
	return sm_http_header_find(list->next, key);
}

#if HTTP_PRINTF

int32_t sm_http_header_printf(sm_http_header_t *list) {
	if (list == NULL)
		return 0;

	printf("Key : %s\tValue : %s\n", list->key, list->value);

	return sm_http_header_printf(list->next);

}
#endif
