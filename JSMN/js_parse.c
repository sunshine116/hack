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

#define PARSER_LEN        64
#define TOKEN_SIZE        200

#define QUERY_LEN         6
#define DIR_LEN           11

extern unsigned char order_display_flag;
extern unsigned char dir_display_flag;
extern unsigned int dir_display_tick;

// static unsigned int UID = 666;
static unsigned int MID = 0;
static char orderId[QUERY_LEN] = {0};
static char DIR[DIR_LEN] = {0};

char js_buf[64];
static char *js_parse_target(char *js_start, char *target, unsigned char *result)
{
	unsigned char j, k;

	memset(js_buf, 0, 64);
	if(!memcmp(js_start, target, strlen(target)))
	{
		j =strlen(target) + 1;
		while(*(js_start + j) != '\"')
		{
			j++;
		}
		k = 0;
		while(*(js_start + j + 1) != '\"')
		{
			js_buf[k] = *(js_start + j + 1);
			j++;
			k++;
		}
		*result = 0;
		return js_buf;
	}
	*result = 1;
	return NULL;
}

void parse_js(char *js)
{
	char *buf;
	unsigned char i, result = 1;

	for(i = 0; i < strlen(js); i++)
	{
		buf = js_parse_target(&js[i], "MID", &result);
		if(result)
			continue;
		MID = atoi(buf);
		printf("MID: %d\r\n", MID);
		MID++;
		break;
	}
	for(i = 0; i < strlen(js); i++)
	{
		buf = js_parse_target(&js[i], "dir", &result);
		if(result)
			continue;
		strcpy(DIR, buf);
		printf("DIR: %s\r\n", DIR);
		break;
	}
	for(i = 0; i < strlen(js); i++)
	{
		buf =js_parse_target(&js[i], "orderId", &result);
		if(result)
			continue;
		strcpy(orderId, buf);
		printf("orderId: %s\r\n", orderId);
		break;
	}
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
