#ifndef COMMON_MQTT_SM_MQTT_CLIENT_H_
#define COMMON_MQTT_SM_MQTT_CLIENT_H_

#include "stdint.h"

#include "sm_config.h"

#if USING_MQTT

typedef enum{
    MQTT_ERROR_FAIL = -1,
    MQTT_ERROR_NO = 0,
    MQTT_ERROR_OPEN,
    MQTT_ERROR_DISCONNECT,
    MQTT_ERROR_CLOSE,
    MQTT_ERROR_CONNECT,
    MQTT_ERROR_NO_DATA,
}sm_mqtt_client_error_t;

typedef enum{
    MQTT_ST_OPEN = 1,
    MQTT_ST_CONNECT,
    MQTT_ST_CONNECTED,
    MQTT_ST_DISCONNECT,
    MQTT_ST_CLOSE,
	MQTT_ST_CLOSED,
}sm_mqtt_client_st_t;

typedef enum{
    MQTT_EVENT_NONE = 0,
    MQTT_EVENT_CONNECTED = 1,
    MQTT_EVENT_DISCONNECTED,
    MQTT_EVENT_ERROR,
    MQTT_EVENT_OPEN,
    MQTT_EVENT_CLOSE,
    MQTT_EVENT_DATA
}sm_mqtt_client_event_t;

typedef struct sm_mqtt_client sm_mqtt_client_t;

typedef void (*sm_mqtt_client_event_handle)(sm_mqtt_client_t *client,sm_mqtt_client_event_t event);

typedef struct 
{
    /* data */
	sm_mqtt_client_error_t (*open)(sm_mqtt_client_t *client,char *host,uint32_t port);
    sm_mqtt_client_error_t (*close)(sm_mqtt_client_t *client);
    sm_mqtt_client_error_t (*connect)(sm_mqtt_client_t *client,char *name,char *user,char *pass);
    sm_mqtt_client_error_t (*disconnect)(sm_mqtt_client_t *client);
    sm_mqtt_client_error_t (*publics)(sm_mqtt_client_t *client,const char *topic,const char* data,uint32_t len,uint8_t qos,uint8_t retain);
    sm_mqtt_client_error_t (*subscribes)(sm_mqtt_client_t *client,const char *topic,uint8_t qos);
    sm_mqtt_client_error_t (*unsubscribes)(sm_mqtt_client_t *client,const char* topic);
    sm_mqtt_client_error_t (*data_iscomming)(sm_mqtt_client_t *client,char* topic,char* data);
//	sm_mqtt_client_error_t (*reconnect)(sm_mqtt_client_t *client);
}sm_mqtt_client_proc_t;


struct sm_mqtt_client
{
    /* data */
    void *arg;
    void *parent;
    sm_mqtt_client_proc_t *m_proc;
    sm_mqtt_client_event_handle event_handle;
    uint32_t num_handle;
    char* host;
    uint32_t port;
    char* client_id;
    char* username;
    char* pass;
    char *topic;
    char *data;
};

/**
 * @brief
 * @param p_mqtt
 * @param state
 */
void sm_mqtt_client_set_state(sm_mqtt_client_t *p_mqtt, sm_mqtt_client_st_t state);
/**
 * @brief
 * @param p_mqtt
 * @return
 */
sm_mqtt_client_st_t sm_mqtt_client_get_state(sm_mqtt_client_t *p_mqtt);

/**
 * @brief
 * @param m_proc
 * @param arg
 * @param host
 * @param port
 * @param user
 * @param pass
 * @return
 */
sm_mqtt_client_t* sm_mqtt_client_init(sm_mqtt_client_proc_t *m_proc,void *arg,char* host,uint32_t port,char* user, char* pass);
/**
 *
 * @param m_proc
 * @param arg
 * @param host
 * @param port
 * @param user
 * @param pass
 * @return
 */
sm_mqtt_client_t* sm_mqtt_client_init_static(sm_mqtt_client_proc_t *m_proc,void *arg,char* host,uint32_t port,char* user, char* pass);

/**
 * @brief
 * @param client
 */
void sm_mqtt_client_deinit(sm_mqtt_client_t *client);
/**
 * @brief
 * @param client
 */
void sm_mqtt_client_process(sm_mqtt_client_t *client);
/**
 * @brief
 * @param client
 * @param handle
 */
void sm_mqtt_client_register_handle(sm_mqtt_client_t *client,sm_mqtt_client_event_handle handle);
/**
 * @brief
 * @param client
 * @param handle
 */
void sm_mqtt_client_unregister_handle(sm_mqtt_client_t *client,sm_mqtt_client_event_handle handle);
/**
 * @brief
 * @param client
 * @param host
 * @param port
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_open(sm_mqtt_client_t *client,char *host,uint32_t port){
    return client->m_proc->open(client,host,port);
}
/**
 * @brief
 * @param client
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_close(sm_mqtt_client_t *client){
    return client->m_proc->close(client);
}
/**
 * @brief
 * @param client
 * @param name
 * @param user
 * @param pass
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_connect(sm_mqtt_client_t *client,char *name,char *user,char *pass){
    return client->m_proc->connect(client,name,user,pass);
}
/**
 * @brief
 * @param client
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_disconnect(sm_mqtt_client_t *client){
    return client->m_proc->disconnect(client);
}
/**
 * @brief
 * @param client
 * @param topic
 * @param data
 * @param len
 * @param qos
 * @param retain
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_publics(sm_mqtt_client_t *client,const char *topic,const char* data,uint32_t len,uint8_t qos,uint8_t retain){
    return client->m_proc->publics(client,topic,data,len,qos,retain);
}
/**
 * @brief
 * @param client
 * @param topic
 * @param qos
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_subscribes(sm_mqtt_client_t *client,const char *topic,uint8_t qos){
    return client->m_proc->subscribes(client,topic,qos);
}
/**
 * @brief
 * @param client
 * @param topic
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_unsubscribes(sm_mqtt_client_t *client,const char* topic){
    return client->m_proc->unsubscribes(client,topic);
}
/**
 * @brief
 * @param client
 * @param topic
 * @param data
 * @return
 */
static inline sm_mqtt_client_error_t sm_mqtt_data_iscomming(sm_mqtt_client_t *client,char* topic,char* data){
    return client->m_proc->data_iscomming(client,topic,data);
}

#endif
#endif
