#include <stdio.h>
#include "poll.h"
#include "sys.h"
#include "hc05.h"
#include "delay.h"
#include "oled.h"
#include "bt_send.h"
#include "usart2.h"
#include "exti.h"
#include "js_parse.h"

extern unsigned char dir_display_flag;
extern unsigned char order_display_flag;

static unsigned int accident_tick = 0;

static unsigned char order_poll_flag = 0xFF;
static unsigned int order_start_poll_tick = 0;

unsigned int dir_display_tick;

static unsigned int temp_start_tick = 0;

//显示ATK-HC05模块的连接状态
void HC05_connect_poll(void)
{
	static unsigned char flag = 0;
	unsigned char i;
	if(HC05_LED)
	{
		if(0 == flag)
		{
			OLED_display(2, 5);
			flag = 1;
			for(i = 0; i < 20; i++)
				delay_ms(100);
		}
	}
	else
	{
		flag = 0;
		OLED_display(0, 4);
		while(!HC05_LED)
		{
			for(i = 0; i < 10; i++)
				delay_ms(100);
		}
	}
}

void accident_sta_poll(void)
{
	if(is_accident_happen())
	{
		if(0xFFFFFFFF - accident_tick >= ACCIDENT_PERIOD/TIME_PER_TICK)
		{
			if(get_tick() - accident_tick >= ACCIDENT_PERIOD/TIME_PER_TICK)
			{
				accident_tick = get_tick();
				clear_accident_flag();
				bt_resp_send(0, 0, 1);
			}
		}
		else
		{
			if(get_tick() >= (ACCIDENT_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - accident_tick)))
			{
				accident_tick = get_tick();
				clear_accident_flag();
				bt_resp_send(0, 0, 1);
			}
		}
	}
}

//0: read the tmp
//1: poll contiune
void order_resp_poll(void)
{
	if(order_poll_flag == 0)
	{
		if(0xFFFFFFFF - order_start_poll_tick >= ORDER_PERIOD/TIME_PER_TICK)
		{
			if(get_tick() - order_start_poll_tick >= ORDER_PERIOD/TIME_PER_TICK)
			{
				bt_send_order(2);
			}
			else if(get_order_result())
			{
				bt_send_order(get_order_result());
			}
		}
		else
		{
			if(get_tick() >= (ORDER_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - accident_tick)))
			{
				bt_send_order(2);
			}
			else if(get_order_result())
			{
				bt_send_order(get_order_result());
			}
		}
	}
}

void temp_upload_poll(void)
{
	if(0xFFFFFFFF - temp_start_tick >= TEMP_PERIOD/TIME_PER_TICK)
	{
		if(get_tick() - temp_start_tick >= TEMP_PERIOD/TIME_PER_TICK)
		{
			bt_resp_send(0, 1, 0);
			OLED_display(3, 255);
			temp_start_tick = get_tick();
			printf("temp display\r\n");
		}
	}else
	{
		if(get_tick() > (TEMP_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - temp_start_tick)))
		{
			bt_resp_send(0, 1, 0);
			OLED_display(3, 255);
			temp_start_tick = get_tick();
			printf("temp display\r\n");
		}
	}
}

void bt_receive_poll(void)
{
	unsigned char reclen=0;
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
}

void dir_display_poll(void)
{
	if(dir_display_flag == 1)
	{
		if(get_tick() - dir_display_tick > DIR_PERIOD/TIME_PER_TICK)
			dir_display_flag = 0;
	}
}

void order_poll_start(void)
{
	order_start_poll_tick = get_tick();
	set_order_flag(0);
	order_poll_flag = 0;
	order_display_flag = 1;
	OLED_display(4, 6);
}

void stop_order_poll(void)
{
	order_poll_flag = 0xFF;
}
