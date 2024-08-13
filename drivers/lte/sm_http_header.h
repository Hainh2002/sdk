/*
 * sm_http_header.h
 *
 *  Created on: Jul 25, 2023
 *      Author: Admin
 */

#ifndef INCLUDE_SM_HTTP_HEADER_H_
#define INCLUDE_SM_HTTP_HEADER_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HTTP_PRINTF 0

typedef struct sm_http_header sm_http_header_t;

struct sm_http_header {
	char *key;
	char *value;
	sm_http_header_t *next;
};

sm_http_header_t* sm_http_header_new(char *key, char *value);
/**
 * @brief
 * @param list
 * @return
 */
sm_http_header_t* sm_http_header_delete(sm_http_header_t *list);
/**
 * @brief
 * @param list
 * @param key
 * @param value
 * @return
 */
int32_t sm_http_header_add(sm_http_header_t *list, char *key, char *value);
/**
 * @brief
 * @param list
 * @param key
 * @return
 */
int32_t sm_http_header_remove(sm_http_header_t *list, char *key);
/**
 * @brief
 * @param list
 * @param key
 * @return
 */
sm_http_header_t* sm_http_header_find(sm_http_header_t *list, char *key);
#if HTTP_PRINTF
int32_t sm_http_header_printf(sm_http_header_t *list);
#endif
#endif /* INCLUDE_SM_HTTP_HEADER_H_ */
