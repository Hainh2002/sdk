#include "sm_ec200s.h"
#include "sm_hal_delay.h"
#include "sm_string_util.h"
#include <string.h>
#include "stdio.h"
#if USING_MODEM
#define MAX_NUM_RETAIN 3

void sm_ec200s_init(sm_ec200s_t *modem, sm_hal_io_t *reset_pin,
		sm_hal_uart_t *driver) {
	sm_modem_init((sm_modem_t*) modem, reset_pin, driver);
	EC200_INIT: sm_modem_hw_reset((sm_modem_t*) modem, 400, 0);

	sm_hal_delay_ms(15000);

	uint32_t retain = 0;

	if (sm_ec200s_sync(modem) != 0) {
		retain++;
		if (retain > 3)
			goto EC200_INIT;
	}
	retain = 0;
	if (sm_ec200s_get_pin(modem) != 0) {
		retain++;
		if (retain > 3)
			goto EC200_INIT;
	}
	retain = 0;
	if (sm_ec200s_get_funtion(modem) != 0) {
		retain++;
		if (retain > 3)
			goto EC200_INIT;
	}
	retain = 0;
	if (sm_ec200s_get_signal(modem) != 0) {
		retain++;
		if (retain > 3)
			goto EC200_INIT;
	}
	retain = 0;
	if (sm_ec200s_get_simNb(modem) != 0) {
		retain++;
		if (retain > 3)
			goto EC200_INIT;
	}
	sm_ec200s_echo_mode(modem, 0);

}
void sm_ec200s_deinit(sm_ec200s_t *modem) {
	sm_modem_deinit((sm_modem_t*) modem);
}

int32_t sm_ec200s_sync(sm_ec200s_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem, "AT/r/n", "OK\r\n",
			"ERROR\r\n", 1000);
}
int32_t sm_ec200s_echo_mode(sm_ec200s_t *modem, uint8_t mode) {
	if (mode == 0)
		return sm_modem_send_cmd((sm_modem_t*) modem, "ATE0/r/n", "OK\r\n",
				"ERROR\r\n", 1000);
	return sm_modem_send_cmd((sm_modem_t*) modem, "ATE1/r/n", "OK\r\n",
			"ERROR\r\n", 1000);
} // mode = 0 : disable echo mode, mode = 1 : enable echo mode
int32_t sm_ec200s_set_pin(sm_ec200s_t *modem, char *pin) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	char buff[128];
	memset(buff, 0, p_modem->buff_length);

	sprintf(buff, "AT+CPIN=%s\r\n", pin);

	return sm_modem_send_cmd((sm_modem_t*) modem, buff, "OK\r\n", "ERROR\r\n",
			1000);
}
int32_t sm_ec200s_get_pin(sm_ec200s_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem, "AT+CPIN?\r\n", "OK\r\n",
			"ERROR\r\n", 1000);
}
int32_t sm_ec200s_get_signal(sm_ec200s_t *modem) {
	int32_t rc = sm_modem_send_cmd((sm_modem_t*) modem, "AT+CSQ\r\n", "OK\r\n",
			"ERROR\r\n", 1000);

	if (rc == 0) {
		sscanf(modem->base.buff, "AT+CSQ\r\r\n+CSQ: %d,%d\r\n\r\nOK\r\n",
				&modem->rssi, &modem->ber);
	}
	return rc;
}
int32_t sm_ec200s_get_name(sm_ec200s_t *modem, char *name) {
	int32_t rc = sm_modem_send_cmd((sm_modem_t*) modem, "AT+GMM\r\n", "OK\r\n",
			"ERROR\r\n", 1000);

	if (rc == 0) {
		memcpy(modem->base.buff + 2, name, 4);
	}
	return rc;
}
int32_t sm_ec200s_get_funtion(sm_ec200s_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem, "AT+CFUN?\r\n", "OK\r\n",
			"ERROR\r\n", 1000);
}
int32_t sm_ec200s_set_apn(sm_ec200s_t *modem, char *apn, char *user, char *pass) {
	char buff[256] = { 0 };
	sprintf(buff, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1\r\n", apn, user, pass);
	return sm_modem_send_cmd((sm_modem_t*) modem, buff, "OK\r\n", "ERROR\r\n",
			1000);
}
int32_t sm_ec200s_get_network_reg(sm_ec200s_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem, "AT+CREG?\r\n", "OK\r\n",
			"ERROR\r\n", 1000);
}

int32_t sm_ec200s_get_simNb(sm_ec200s_t *modem) {
	int32_t rc = sm_modem_send_cmd((sm_modem_t*) modem, "AT+CIMI\r\n", "OK\r\n",
			"ERROR\r\n", 1000);
	if (rc == 0) {
		sscanf(modem->base.buff, "AT+CIMI\r\r\n%s\r\n\r\nOK\r\n",
				(char*) modem->IMSI);
		strncpy(modem->simNb, modem->IMSI + 7, 8);
	}
	return rc;
}

int32_t sm_ec200s_get_ip(sm_ec200s_t *modem) {
	if (sm_modem_send_cmd(&modem->base, "AT+QIACT=1\r\n", "OK\r\n", "ERROR\r\n",
			10000) == 0) {
		if (sm_modem_send_cmd(&modem->base, "AT+QIACT?\r\n", "OK\r\n",
				"ERROR\r\n", 10000) == 0) {
			char *str = strstr(modem->base.buff, "+QIACT");
			sscanf(str, "+QIACT: 1,1,1,\"%d.%d.%d.%d\"", &modem->ip[0],
					&modem->ip[1], &modem->ip[2], &modem->ip[3]);
			sm_modem_send_cmd(&modem->base, "AT+QIDEACT=1\r\n", "OK\r\n",
					"ERROR\r\n", 10000);
			return 0;
		}
	}
	return -1;
}
int32_t sm_ec200s_config_mqtt(sm_ec200s_t *modem){
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	char buff[512] = { 0 };
	memset(buff, 0, 512);
	sprintf(buff, "AT+QMTCFG=\"recv/mode\",1,1,0\r\n");
	return sm_modem_send_cmd(p_modem, buff, "OK\r\n","ERROR\r\n",1000);
}
int32_t sm_ec200s_open_mqtt(sm_ec200s_t *modem, char *host, uint32_t port) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	char buff[512] = { 0 };
	memset(buff, 0, 512);
	sprintf(buff, "AT+QMTOPEN=1,\"%s\",%d\r\n", host, (unsigned int) port);
	return sm_modem_send_cmd(p_modem, buff, "OK\r\n\r\n+QMTOPEN: 1,0\r\n",
			"ERROR\r\n", 2000);

}
int32_t sm_ec200s_connect_mqtt(sm_ec200s_t *modem, char *device_name,
		char *user, char *pass) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;

	char buff[512] = { 0 };
	memset(buff, 0, 512);

	if (user != NULL && pass != NULL) {
		sprintf(buff, "AT+QMTCONN=1,\"%s\",%s,%s\r\n", device_name, user, pass);
	} else {
		sprintf(buff, "AT+QMTCONN=1,\"%s\"\r\n", device_name);
	}
	return sm_modem_send_cmd(p_modem, buff, "OK\r\n\r\n+QMTCONN: 1,0,0\r\n",
			"ERROR\r\n", 2000);
}
int32_t sm_ec200s_disconnect_mqtt(sm_ec200s_t *modem) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	return sm_modem_send_cmd(p_modem, "AT+QMTDISC=1\r\n",
			"OK\r\n\r\n+QMTDISC: 1,0\r\n", "ERROR\r\n", 2000);
}
int32_t sm_ec200s_close_mqtt(sm_ec200s_t *modem) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	return sm_modem_send_cmd(p_modem, "AT+QMTCLOSE=1\r\n",
			"OK\r\n\r\n+QMTCLOSE: 1,0\r\n", "ERROR\r\n", 2000);
}
int32_t sm_ec200s_public_mqtt(sm_ec200s_t *modem, char *topic, char *data,
		uint32_t len, int qos, int retain) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;

	char buff[512] = { 0 };
	memset(buff, 0, 512);
	sprintf(buff, "AT+QMTPUBEX=1,1,%d,%d,%s,%lu\r\n", qos, retain, topic, len);

	if (sm_modem_send_cmd(p_modem, buff, ">", "ERROR\r\n", 2000) != 0) {
		return -1;
	}
	return sm_modem_send_cmd(p_modem, data, "OK\r\n\r\n+QMTPUBEX: 1,1,0",
			"ERROR\r\n", 2000);
}
int32_t sm_ec200s_subscribes_mqtt(sm_ec200s_t *modem, char *topic, int qos) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	char buff[512] = { 0 };
	memset(buff, 0, 512);
	sprintf(buff, "AT+QMTSUB=1,1,%s,%d\r\n", topic, qos);
	return sm_modem_send_cmd(p_modem, buff, "OK\r\n\r\n+QMTSUB: 1,1,0",
			"ERROR\r\n", 2000);
}
int32_t sm_ec200s_unsubscribes_mqtt(sm_ec200s_t *modem, char *topic) {
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	char buff[512] = { 0 };
	memset(buff, 0, 512);
	sprintf(buff, "AT+QMTUNS=1,1,\"%s\"", topic);
	return sm_modem_send_cmd(p_modem, buff, "OK\r\n\r\n+QMTUNS: 1,1,0",
			"ERROR\r\n", 2000);
}

char *token = NULL;
int32_t sm_ec200s_recive_mqtt_data(sm_ec200s_t* modem){
	sm_modem_t* p_modem = (sm_modem_t*) modem;
	char buff[512] = {0};
	memset(buff, 0, 512);
	sprintf(buff, "AT+QMTRECV?\r\n");
	sm_modem_send_cmd(p_modem, buff, "OK\r\n","OK\r\n", 2000);

	if(strstr(p_modem->buff,"+QMTRECV: 1,0,0,0,0,0") != NULL) return -1;

	if(strstr(p_modem->buff,"+QMTRECV") != NULL){

		token = strtok(p_modem->buff,"\r\n");
		int buff_index[5] = {0,0,0,0,0};
		while(token != NULL){
			sscanf(token, "+QMTRECV: 1,%d,%d,%d,%d,%d\r",
					&buff_index[0], &buff_index[1], &buff_index[2],
					&buff_index[3], &buff_index[4]);
			for (unsigned int i = 0; i < 5; i++) {
				if (buff_index[i] == 1)
					return (int)i;
			}
			token = strtok(NULL,"\r\n");
		}
	}
	return -1;
}
int32_t sm_ec200s_data_iscomming_mqtt(sm_ec200s_t *modem, char *topic,
		char *data) {

	int buff_index = sm_ec200s_recive_mqtt_data(modem);

	if(buff_index == -1){
		return - 1;
	}

	sm_modem_t* p_modem = (sm_modem_t*) modem;
	char buff[512] = {0};
	memset(buff, 0, 512);
	sprintf(buff, "AT+QMTRECV=1,%d\r\n",buff_index);
	if(sm_modem_send_cmd(p_modem, buff, "OK\r\n","ERROR\r\n", 5000) != 0) return -1;
	parse_mqtt(p_modem->buff, topic, data);

	return 0;
}



int32_t sm_ec200s_data_iscomming_http(sm_ec200s_t* modem, char* data){
	char c = 0;
	sm_modem_t *p_modem = (sm_modem_t*) modem;
	if (sm_modem_read(p_modem, &c, 1) == 1) {
		uint32_t p_index = 0;
		uint32_t p_len = 0;
		memset(p_modem->buff, 0, p_modem->buff_length);
		uint32_t time = 0;
		uint32_t timeout = 2000;
		while(1){
			p_len = (uint32_t) sm_modem_read(p_modem, p_modem->buff + p_index,
					p_modem->buff_length - p_index);
			p_index += p_len;
			if ((strstr(p_modem->buff, "\"\r\n") != NULL)
					&& (strstr(p_modem->buff, "+QHTTPGET: 0,206") != NULL)) {
				return 2;
			}
			time++;
			if (time > timeout)
				return -1;
			sm_hal_delay_ms(1);
		}
	}
	return 0;
}
int32_t sm_ec200_http_open(sm_ec200s_t *modem, char *host, int port) {
	char cmd[256];
	sprintf(cmd, "AT+QIOPEN=1,1,\"TCP\",\"%s\",%d,0,0\r\n", host, port);
	return sm_modem_send_cmd((sm_modem_t*) modem, cmd, "OK\r\n\r\n+QIOPEN: 1,0",
			"ERROR", 10000);
}
int32_t sm_ec200_http_close(sm_ec200s_t *modem) {
	return sm_modem_send_cmd((sm_modem_t*) modem, "AT+QICLOSE=1\r\n", "OK\r\n",
			"ERROR\r\n", 5000);
}
int32_t sm_ec200_http_write(sm_ec200s_t *modem, char *data, uint32_t len) {
	char cmd[128];
	if (len != strlen(data))
		len = strlen(data);
	sprintf(cmd, "AT+QISEND=1,%d\r\n", strlen(data));

	if (sm_modem_send_cmd((sm_modem_t*) modem, cmd, ">", "ERROR", 5000) == 0) {
		return sm_modem_send_cmd((sm_modem_t*) modem, data, "SEND OK", "ERROR",
				5000);
	}

	return -1;
}
int32_t sm_ec200s_http_read(sm_ec200s_t *modem, char *data) {
	char cmd[128];
	sprintf(cmd, "AT+QHTTPREAD=80\r\n");
    if (sm_modem_send_cmd ((sm_modem_t*) modem, cmd, "+QHTTPREAD: 0\r\n", "+CME ERROR",
			5000) == 0) {
        strcpy (data, modem->base.buff);
		return 0;
	}
    return -1;
}
int32_t sm_ec200s_http_config(sm_ec200s_t* modem, const char* paramater, int value){
	char cmd[128];
	sprintf(cmd, "AT+QHTTPCFG=\"%s\",%d",paramater, value);
	return sm_modem_send_cmd((sm_modem_t*) modem , cmd, "OK\r\n", "+CME ERROR", 2000);
}
int32_t sm_ec200s_set_http_url(sm_ec200s_t *modem, char *url){
	char buff[512] = { 0 };
	memset(buff, 0, 512);

	sprintf(buff, "AT+QHTTPURL=%d,%d\r\n", (int)strlen(url), 60);

    if (sm_modem_send_cmd ((sm_modem_t*) modem, buff, "CONNECT", "+CME ERROR\r\n", 2000) != 0)
    {
		return -1;
	}
	return sm_modem_send_cmd((sm_modem_t*) modem, url, "OK\r\n",
			"+CME ERROR\r\n", 2000);
}

int32_t sm_ec200s_http_get_range(sm_ec200s_t* modem,uint32_t start, uint32_t range){
	char buff[512] = {0};
	memset(buff, 0, 512);
	sprintf(buff, "AT+QHTTPGETEX=2,%d,%d\r\n", start, range);
	return sm_modem_send_cmd((sm_modem_t*) modem, buff, "+QHTTPGET: 0", "+CME ERROR\r\n", 2000);
}

int32_t sm_ec200s_get_time_network(sm_ec200s_t* modem, struct tm *p_time){
	char cmd[128] = {0};
	memset(cmd, 0, 128);
	sprintf(cmd, "AT+QLTS=2\r\n");

    if (sm_modem_send_cmd ((sm_modem_t*) modem, cmd, "OK", "ERROR", 1000) != 0)
    {
		return -1;
	}
    char* str_time = strstr(modem->base.buff,"\"");
    if(str_time == NULL){
    	return -1;
    }
    sscanf(str_time,"\"%d/%d/%d,%d:%d:%d+",&p_time->tm_year,&p_time->tm_mon,&p_time->tm_mday,&p_time->tm_hour,&p_time->tm_min,&p_time->tm_sec);
    return 0;
}
#endif
