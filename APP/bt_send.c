#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "bt_send.h"
#include "oled.h"
#include "poll.h"
#include "delay.h"
#include "DS18B20.h"

extern unsigned char order_display_flag;

static unsigned int send_tick;

void bt_send_order(unsigned char order_response)
{
	bt_resp_send(order_response, 1, 0);
	stop_order_poll();
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

	if(get_tick() - send_tick < 3000/TICK_PERIOD)
	{
		for(i = 0; i < 30; i++)
			delay_ms(100);
		send_tick = get_tick();
	}
	else
	{
		send_tick = get_tick();
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
		// strcpy(order_buf, orderId);
	}
	else if(order == 2)
	{
		order_display_flag = 0;
		memset(order_buf, '-', 1);
		// strcpy(order_buf + 1, orderId);
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
	// u2_printf("{\"UID\":\"%d\",\"MID\":\"%d\",\"data\":{\"orderId\":\"%s\",\"temp\":\"%s\",\"accident\":\"%d\"}}", UID, MID, order_buf, Temp_buf, accident);
	printf("!!!!order_buf!!!!!!: %s\r\n", order_buf);
	// if(order != NULL)
	// 	memset(orderId, 0, QUERY_LEN);
	free(order_buf);
	free(Temp_buf);
	return 0;
}

