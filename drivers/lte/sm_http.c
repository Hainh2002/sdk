/*
 * http_client.c
 *
 *  Created on: Jul 17, 2023
 *      Author: Admin
 */

#include <sm_http.h>
#include <stdio.h>
#include <string.h>

static const char *http_method_maping[8] =
{ "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE" };

static void sm_http_client_packet(sm_http_client_t *client);

static void sm_http_client_packet(sm_http_client_t *client)
{
    sm_http_client_config_t *p_config = client->config;
    sprintf (p_config->header, "%s /%s HTTP/1.1\r\n", http_method_maping[p_config->method], p_config->path);
    uint32_t len = strlen (p_config->header);

    sm_http_header_t *node = p_config->header_list;

    while (node != NULL)
    {
        sprintf (p_config->header + len, "%s: %s\r\n", node->key, node->value);
        node = node->next;
        len = strlen (p_config->header);
    }

    if (strlen (client->config->content) > 0)
    {
        client->config->content_len = strlen (client->config->content);
		sprintf(p_config->header + len, "Content-Length: %d\r\n\r\n%s\r\n",
				(unsigned int) client->config->content_len,
				client->config->content);
        len = strlen (p_config->header);
    }
    else
    {
        sprintf (p_config->header + len, "\r\n");
        len = strlen (p_config->header);
    }
    //printf(p_config->buff);

}

/**
 * @brief
 * @param client
 * @param config
 * @param func
 * @return
 */
int32_t sm_http_client_init(sm_http_client_t *client, sm_http_client_config_t *config, sm_http_client_funtion_t *func)
{
    client->config = config;
    client->funtion = func;

    client->config->header = client->config->buff;
    if (client->config->keep_alive_interval == 0)
        client->config->keep_alive_interval = 5000;
    if (client->config->url != NULL)
    {
        char *host = strstr (client->config->url, "://");

        if (host != NULL)
        {
            host[0] = '\0';
            host[1] = '\0';
            host[2] = '\0';
            host += 3;
            client->config->host = host;
            char *path = strstr (host, "/");
            if (path != NULL)
            {
                path[0] = '\0';
                client->config->path = path;
            }
        }

    }
    client->config->header_list = sm_http_header_new ("Host", (char*) client->config->host);
    return 0;
}
int32_t sm_http_client_deinit(sm_http_client_t *client)
{

    client->funtion = NULL;
    client->config->buff = NULL;
    client->config->header = NULL;
    client->config->content = NULL;
    client->config->content_len = 0;
    client->config->header_list = sm_http_header_delete (client->config->header_list);
    return 0;
}
int32_t sm_http_client_perform(sm_http_client_t *client)
{

    sm_http_client_packet (client);
    if (client->funtion != NULL)
    {
        if (client->funtion->open != NULL)
        {
            if (client->funtion->open (client) == 0)
            {
                if (client->funtion->write != NULL)
                {
                    if (client->funtion->write (client, (uint8_t*) client->config->buff, strlen (client->config->buff))
                            == 0)
                    {
                        if (client->funtion->read != NULL)
                        {

                            int timeout_ms = client->config->keep_alive_interval;
                            int time = 0;
                            while (time < timeout_ms)
                            {
                                uint32_t len = client->funtion->read (client, (uint8_t*) client->config->buff,
                                                                      client->config->buff_len);
                                if (len > 0)
                                {
                                    sm_http_client_parser (client, client->config->buff);
                                    return 0;
                                }
                                time += 100;
                                sm_bsp_delay_ms (100);
                            }
                        }
                    }

                }
            }
        }
    }
    return -1;
}

/**
 * @brief
 * @param client
 * @param key
 * @param value
 * @return
 */
int32_t sm_http_client_set_header(sm_http_client_t *client, char *key, char *value)
{
    sm_http_header_t *header = sm_http_header_find (client->config->header_list, key);

    if (header != NULL)
    {
        header->value = value;
        return 0;
    }

    return sm_http_header_add (client->config->header_list, key, value);;
}
/**
 * @brief
 * @param client
 * @param key
 * @param value
 * @return
 */
int32_t sm_http_client_get_header(sm_http_client_t *client, const char *key, char **value)
{

	sm_http_header_t *header = sm_http_header_find(client->config->header_list,
			(char*) key);
    if (header != NULL)
    {
        value[0] = header->value;

        return 0;
    }
    return -1;
}
/**
 * @brief
 * @param client
 * @param key
 * @return
 */
int32_t sm_http_client_delete_header(sm_http_client_t *client, char *key)
{
    return sm_http_header_remove (client->config->header_list, key);
}
/**
 * @brief
 * @param client
 * @param url
 * @return
 */
int32_t sm_http_client_set_url(sm_http_client_t *client, char *url)
{
    client->config->url = url;
    // find host

    // find path

    return 0;
}
/**
 * @brief
 * @param client
 * @param method
 * @return
 */
int32_t sm_http_client_set_method(sm_http_client_t *client, sm_http_method_t method)
{

    client->config->method = method;
    sm_http_client_config_t *p_config = client->config;
    sprintf (p_config->method_str, "%s /%s HTTP/1.1\r\n", http_method_maping[p_config->method], p_config->path);
    return 0;
}
/**
 * @brief
 * @param client
 * @return
 */
sm_http_error_code_t sm_http_client_get_status_code(sm_http_client_t *client)
{

    char *line = client->config->buff;

    if (strstr (line, "HTTP") != NULL)
    {
        unsigned int status_code = 0;
        sscanf (line, "HTTP/1.1 %d ", &status_code);
        client->config->error_code = status_code;
        return status_code;
    }
    return -1;
}
/**
 * @brief
 * @param client
 * @return
 */
uint64_t sm_http_client_get_content_length(sm_http_client_t *client)
{
    return (uint64_t) client->config->content_len;
}
int32_t sm_http_client_parser(sm_http_client_t *client, char *data)
{
    client->config->header = data;
    client->config->content = strstr (data, "\r\n\r\n");
	if (client->config->content != NULL) {
		client->config->content[0] = 0;
		client->config->content[1] = 0;
		client->config->content[2] = 0;
		client->config->content[3] = 0;
		client->config->content += 4;
	}
	char *line = client->config->header;
	uint32_t length = strlen(line);
	uint32_t len = 0;
	uint32_t _index = 0;

	char *key = NULL;
	char *value = NULL;

	if (client->config->header_list == NULL) {
		client->config->header_list = sm_http_header_new("Parser", "http");
	}

    sm_http_client_get_status_code (client);
	
	while (len < length) {
		if (line[_index] == '\n') {
			line[_index] = 0;
			line[_index - 1] = 0;
#if HTTP_PRINTF
			printf(line);
			printf("\n");
#endif
			if (strstr(line, "HTTP") != NULL) {

			}
			else {
				key = line;
				value = strstr(line, ": ");
				value[0] = '\0';
				value[1] = '\0';
				value += 2;
				sm_http_header_add(client->config->header_list, key, value);
			}
			line += (_index + 1);
			len += _index;
			_index = 0;
		}
		_index++;
	}
#if HTTP_PRINTF
	sm_http_header_printf(config->header_list);
#endif
    return 0;
}
