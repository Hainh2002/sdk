/*
 * http_client.h
 *
 *  Created on: Jul 17, 2023
 *      Author: Admin
 */

#ifndef COMMON_HTTP_HTTP_CLIENT_H_
#define COMMON_HTTP_HTTP_CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sm_http_header.h>

typedef enum {
	HTTP_GET = 0,
	HTTP_HEAD,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_CONNECT,
	HTTP_OPTIONS,
	HTTP_TRACE,
} sm_http_method_t;

typedef enum {
	/* 2xx - Success */
	HttpStatus_Ok = 200,

	/* 3xx - Redirection */
	HttpStatus_MultipleChoices = 300,
	HttpStatus_MovedPermanently = 301,
	HttpStatus_Found = 302,
	HttpStatus_SeeOther = 303,
	HttpStatus_TemporaryRedirect = 307,
	HttpStatus_PermanentRedirect = 308,

	/* 4xx - Client Error */
	HttpStatus_BadRequest = 400,
	HttpStatus_Unauthorized = 401,
	HttpStatus_Forbidden = 403,
	HttpStatus_NotFound = 404,

	/* 5xx - Server Error */
	HttpStatus_InternalError = 500
} sm_http_error_code_t;


typedef struct sm_http_client sm_http_client_t;

typedef struct sm_http_client_config sm_http_client_config_t;

typedef struct {
	int32_t (*open)(sm_http_client_t *client);
	int32_t (*close)(sm_http_client_t *client);
	int32_t (*write)(sm_http_client_t *client, uint8_t *data, uint32_t len);
    uint32_t (*read)(sm_http_client_t *client, uint8_t *data, uint32_t len);

} sm_http_client_funtion_t;

struct sm_http_client_config {
	const char *url; /*!< HTTP URL, the information on the URL is most important, it overrides the other fields below, if any */
	const char *host; /*!< Domain or IP as string */
	int port; /*!< Port to connect, default depend on esp_http_client_transport_t (80 or 443) */
	const char *username; /*!< Using for Http authentication */
	const char *password; /*!< Using for Http authentication */
	const char *path; /*!< HTTP Path, if not set, default is `/` */
	const char *query; /*!< HTTP query */
	const char *user_agent; /*!< The User Agent string to send with HTTP requests */
	sm_http_method_t method; /*!< HTTP Method */
	int timeout_ms; /*!< Network timeout in milliseconds */
	int max_redirection_count; /*!< Max number of redirections on receiving HTTP redirect status code, using default value if zero*/
	void *user_data; /*!< HTTP user_data context */
	bool keep_alive_enable; /*!< Enable keep-alive timeout */
	int keep_alive_idle; /*!< Keep-alive idle time. Default is 5 (second) */
	int keep_alive_interval; /*!< Keep-alive interval time. Default is 5 (second) */
	int keep_alive_count; /*!< Keep-alive packet retry send count. Default is 3 counts */
	sm_http_error_code_t error_code;
	char *buff;
    uint32_t buff_len;
	char *header;
	char *method_str;
	uint32_t header_len;
	char *content;
	uint64_t content_len;
	sm_http_header_t *header_list;
	float http_ver;
	char http_error[50];
};
struct sm_http_client {
	sm_http_client_config_t *config;
	sm_http_client_funtion_t *funtion;
};

/**
 * @brief
 * @param client
 * @param config
 * @param func
 * @return
 */
int32_t sm_http_client_init(sm_http_client_t *client,
		sm_http_client_config_t *config, sm_http_client_funtion_t *func);
int32_t sm_http_client_deinit(sm_http_client_t *client);
/**
 * @brief
 * @param client
 * @param key
 * @param value
 * @return
 */
int32_t sm_http_client_set_header(sm_http_client_t *client, char *key,
		char *value);
/**
 * @brief
 * @param client
 * @param key
 * @param value
 * @return
 */
int32_t sm_http_client_get_header(sm_http_client_t *client, const char *key,
		char **value);
/**
 * @brief
 * @param client
 * @param key
 * @return
 */
int32_t sm_http_client_delete_header(sm_http_client_t *client, char *key);
/**
 * @brief
 * @param client
 * @param url
 * @return
 */
int32_t sm_http_client_set_url(sm_http_client_t *client, char *url);
/**
 * @brief
 * @param client
 * @param method
 * @return
 */
int32_t sm_http_client_set_method(sm_http_client_t *client,
		sm_http_method_t method);
/**
 * @brief
 * @param client
 * @return
 */
sm_http_error_code_t sm_http_client_get_status_code(sm_http_client_t *client);
/**
 * @brief
 * @param client
 * @return
 */
uint64_t sm_http_client_get_content_length(sm_http_client_t *client);
/**
 * @brief
 * @param client
 * @param data
 * @return
 */
int32_t sm_http_client_parser(sm_http_client_t *client, char *data);
/**
 * @brief
 * @param client
 * @return
 */
int32_t sm_http_client_perform(sm_http_client_t *client);

/**
 * @brief
 * @param client
 * @return
 */
static inline int32_t sm_http_client_open(sm_http_client_t *client)
{
    return client->funtion->open (client);
}
/**
 * @brief
 * @param client
 * @return
 */
static inline int32_t sm_http_client_close(sm_http_client_t *client)
{
    return client->funtion->close (client);
}
/**
 * @brief
 * @param client
 * @param data
 * @param len
 * @return
 */
static inline int32_t sm_http_client_write(sm_http_client_t *client, uint8_t *data, uint32_t len)
{
    return client->funtion->write (client, data, len);
}
/**
 * @brief
 * @param client
 * @param data
 * @param len
 * @return
 */
static inline uint32_t sm_http_client_read(sm_http_client_t *client, uint8_t *data, uint32_t len)
{
    return client->funtion->read (client, data, len);
}

#endif /* COMMON_HTTP_HTTP_CLIENT_H_ */
