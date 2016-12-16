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

#define PARSER_LEN        64
#define TOKEN_SIZE        200

#define QUERY_LEN         6
#define DIR_LEN           11

extern unsigned char order_display_flag;
extern unsigned char dir_display_flag;
extern unsigned int tick;

static unsigned int dir_display_tick;
static unsigned int send_tick;

static unsigned int UID = 666;
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
	dir_display_tick = tick;
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
		order_resp_start();
	}
}

unsigned char bt_receive(void)
{
	unsigned char ret = 1, reclen=0;
	if(USART2_RX_STA&0X8000)
	{
		printf("Receive: ");
 		reclen=USART2_RX_STA&0X7FFF;
		USART2_RX_BUF[reclen]=0;
		printf("%s\r\n",USART2_RX_BUF);
		parse_js((char *)USART2_RX_BUF);
		process_server_cmd();
		USART2_RX_STA=0;
	}
	return ret;
}
//order: 	0: 	NULL
//			1:	yes
//			2:	no
//Temp:		0:	NULL
//			1:	temperature
//accident:	0:	NULL
//			1:	accident
unsigned char bt_resp_send(unsigned char order, unsigned char Temp, unsigned char accident)
{
	char* Temp_buf = NULL, *order_buf = NULL;
	unsigned char Temp_buf_len = 1, order_buf_len = 3, i;

	if(tick - send_tick < 3000/TICK_PERIOD)
	{
		for(i = 0; i < 30; i++)
			delay_ms(100);
		send_tick = tick;
	}
	else
	{
		send_tick = tick;
	}
	if(order != NULL)
		order_buf_len = 64;
	order_buf = malloc(order_buf_len);
	if(order_buf == NULL)
	{
		OLED_print_error("order_buf error!");
		return 1;
	}
	memset(order_buf, 0, order_buf_len);
	if(order == 1)
	{
		order_display_flag = 0;
		strcpy(order_buf, orderId);
	}
	else if(order == 2)
	{
		order_display_flag = 0;
		memset(order_buf, '-', 1);
		strcpy(order_buf + 1, orderId);
	}
		
	if(Temp == 1)
		Temp_buf_len = 7;
	Temp_buf = malloc(Temp_buf_len);
	if(Temp_buf == NULL)
	{
		OLED_print_error("Temp_buf error!");
		return 1;
	}
	memset(Temp_buf, 0, Temp_buf_len);
	if(Temp == 1)
		Temp_string_get(Temp_buf);
	MID++;
	u2_printf("{\"UID\":\"%d\",\"MID\":\"%d\",\"data\":{\"orderId\":\"%s\",\"temp\":\"%s\",\"accident\":\"%d\"}}", UID, MID, order_buf, Temp_buf, accident);
	printf("!!!!order_buf!!!!!!: %s\r\n", order_buf);
	if(order != NULL)
		memset(orderId, 0, QUERY_LEN);
	free(order_buf);
	free(Temp_buf);
	return 0;
}

void dir_display_poll(void)
{
	if(dir_display_flag == 1)
	{
		if(tick - dir_display_tick > POLL_PERIOD/TICK_PERIOD)
			dir_display_flag = 0;
	}
}
