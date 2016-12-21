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
#include "jsmn.h"

#define ORDER_LEN         12
#define DIR_LEN           11

extern unsigned char order_display_flag;
extern unsigned char dir_display_flag;
extern unsigned int dir_display_tick;

static char UID[11] = "666";
static char MID[11] = "0"; 
static char orderId[ORDER_LEN] = {0};
static char DIR[DIR_LEN] = {0};

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

unsigned char parse_js(char *js)
{
	int i, r;
	jsmn_parser p;
	jsmntok_t t[64];

	jsmn_init(&p);
	r = jsmn_parse(&p, js, strlen(js), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\r\n", r);
		return 1;
	}
	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return 1;
	}

	/* Loop over all keys of the root object */
	for (i = 1; i < r; i++) {
		if (jsoneq(js, &t[i], "UID") == 0) {
			sprintf(UID, "%.*s", t[i+1].end-t[i+1].start,
					js + t[i+1].start);
			i++;
		} else if (jsoneq(js, &t[i], "MID") == 0) {
			sprintf(MID, "%.*s", t[i+1].end-t[i+1].start,
					js + t[i+1].start);
			i++;
		} else if (jsoneq(js, &t[i], "dir") == 0) {
			sprintf(DIR, "%.*s", t[i+1].end-t[i+1].start,
					js + t[i+1].start);
			i++;
		} else if (jsoneq(js, &t[i], "orderId") == 0) {
			sprintf(orderId, "%.*s", t[i+1].end-t[i+1].start,
					js + t[i+1].start);
			i++;
		}
	}
	return 0;
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
		if(0 != is_order_poll())
		{
			order_poll_start();
		}
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

char *js_compose(unsigned char order, unsigned char Temp, unsigned char accident)
{
	char accident_buf[2], Temp_buf[7], orderId_tmp[ORDER_LEN];
	char *js_buf;

	js_buf = malloc(JS_BUF_LEN);
	if(js_buf == NULL)
	{
		printf("js buf malloc error!\r\n");
		return NULL;
	}

	if(order == 1)
	{
		order_display_flag = 0;
		memcpy(orderId_tmp, orderId, ORDER_LEN);
	}
	else if(order == 2)
	{
		order_display_flag = 0;
		memcpy(orderId_tmp, orderId, ORDER_LEN);
		atona(orderId_tmp);
	}
	else
	{
		memset(orderId_tmp, 0, ORDER_LEN);
	}

	if(Temp == 1)
	{
		Temp_string_get(Temp_buf);
	}
	else
	{
		memset(Temp_buf, 0, 7);
	}

	if(accident == 1)
	{
		strcpy(accident_buf, "1");
	}
	else
	{
		strcpy(accident_buf, "0");
	}
	sprintf(js_buf, "{\"UID\":\"%s\",\"MID\":\"%s\",\"data\":{\"orderId\":\"%s\",\"temp\":\"%s\",\"accident\":\"%s\"}}", UID, MID, orderId_tmp, Temp_buf, accident_buf);

	return js_buf;
}

void reset_orderId(void)
{
	memset(orderId, 0, ORDER_LEN);
}
