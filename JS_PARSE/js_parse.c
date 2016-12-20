#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys.h"
#include "js_parse.h"
#include "oled.h"
#include "DS18B20.h"
#include "usart2.h"
#include "exti.h"
#include "delay.h"
#include "poll.h"
#include "cJSON.h"

#define PARSER_LEN        64
#define TOKEN_SIZE        200

#define ORDER_LEN         12
#define DIR_LEN           11

extern unsigned char order_display_flag;
extern unsigned char dir_display_flag;
extern unsigned int dir_display_tick;

static char UID[11] = "666";
static char MID[11]; 
static char orderId[ORDER_LEN] = {0};
static char DIR[DIR_LEN] = {0};

void parse_js(char *js)
{
	cJSON *receive_json = cJSON_Parse(js), *data;

	memset(UID, 0, 11);
	memset(MID, 0, 11);
	memset(DIR, 0, DIR_LEN);
	memset(orderId, 0, ORDER_LEN);

	strcpy(UID, cJSON_GetObjectItem(receive_json, "UID")->valuestring);
	strcpy(MID, cJSON_GetObjectItem(receive_json, "MID")->valuestring);

	data = cJSON_GetObjectItem(receive_json, "data");
	strcpy(DIR , cJSON_GetObjectItem(data, "dir")->valuestring);
	strcpy(orderId, cJSON_GetObjectItem(data, "orderId")->valuestring);

	cJSON_Delete(receive_json);

	printf("UID: %s, MID: %s\r\nDIR: %s, orderId: %s\r\n", UID, MID, DIR, orderId);
}

void process_server_cmd(void)
{
	dir_display_flag = 1;
	dir_display_tick = get_tick();
	if(0 == strcmp(DIR, "right"))
	{
		OLED_display(1, 3);
	}
	else if(0 == strcmp(DIR, "left"))
	{
		OLED_display(1, 2);
	}
	else if(0 == strcmp(DIR, "forward"))
	{
		OLED_display(1, 0);
	}
	else if(0 == strcmp(DIR, "turn around"))
	{
		OLED_display(1, 1);
	}else
	{
		dir_display_flag = 0;
	}
	memset(DIR, 0, DIR_LEN);

	if(0 != strcmp(orderId, ""))
	{
		order_poll_start();
	}
}

static void atona(char *buf)
{
	unsigned char len = 0;

	for(len = strlen(buf); len > 0; len--)
	{
		buf[len] = buf[len - 1];
	}
	buf[0] = '-';
}
//order: 	0: 	NULL
//			1:	yes
//			2:	no
//Temp:		0:	NULL
//			1:	temperature
//accident:	0:	NULL
//			1:	accident
void js_compose(unsigned char order, unsigned char Temp, unsigned char accident)
{
	cJSON *send_json, *data;
	char accident_buf[2], Temp_buf[7];

	send_json = cJSON_CreateObject();
	cJSON_AddStringToObject(send_json, "UID", UID);
	cJSON_AddStringToObject(send_json, "MID", MID);
	cJSON_AddItemToObject(send_json, "data", data = cJSON_CreateObject());
	if(order == 1)
	{
		order_display_flag = 0;
	}
	else if(order == 2)
	{
		order_display_flag = 0;
		atona(orderId);
	}
	cJSON_AddStringToObject(data, "orderId", orderId);
	if(Temp == 1)
		Temp_string_get(Temp_buf);
	cJSON_AddStringToObject(data, "temp", Temp_buf);
	if(accident == 1)
	{
		strcpy(accident_buf, "1");
	}
	else
	{
		strcpy(accident_buf, "0");
	}
	cJSON_AddStringToObject(data, "accident", accident_buf);
	printf("send json: %s\r\n", cJSON_Print(send_json));
}
