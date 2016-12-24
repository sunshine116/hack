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
#include "display.h"

extern unsigned char dir_display_flag;
extern unsigned char order_display_flag;

static unsigned char order_poll_flag = 0xFF;
static unsigned int order_start_poll_tick = 0;

unsigned int dir_display_tick;

//显示ATK-HC05模块的连接状态
void HC05_connect_poll(void)
{
	if(!HC05_LED)
	{
		change_display_to(BT_STATUS, BT_DISCONN, 0);
		while(!HC05_LED)
			delay_ms(3000);
		change_display_to(BT_STATUS, BT_CONNECT, 0);
	}
}

void accident_sta_poll(void)
{
	static unsigned int accident_tick = 0;

	if(is_accident_happen())
	{
		if(0xFFFFFFFF - accident_tick >= ACCIDENT_PERIOD/TIME_PER_TICK)
		{
			if(get_tick() - accident_tick >= ACCIDENT_PERIOD/TIME_PER_TICK)
			{
				accident_tick = get_tick();
				clear_accident_flag();
				add_send_package(0, 0, 1);
			}
		}
		else
		{
			if(get_tick() >= (ACCIDENT_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - accident_tick)))
			{
				accident_tick = get_tick();
				clear_accident_flag();
				add_send_package(0, 0, 1);
			}
		}
	}
}

void order_poll_start(void)
{
	order_start_poll_tick = get_tick();
	set_order_flag(0);
	order_poll_flag = 0;
	order_display_flag = 1;
	change_display_to(ORDER_STATUS, 0xFF, 0);
}

void stop_order_poll(void)
{
	order_poll_flag = 0xFF;
}

//0: poll is working 0xFF: poll stop
unsigned char is_order_poll(void)
{
	return order_poll_flag;
}

//0: read the tmp
//1: poll contiune
void order_poll(void)
{
	if(order_poll_flag == 0)
	{
		if(0xFFFFFFFF - order_start_poll_tick >= ORDER_PERIOD/TIME_PER_TICK)
		{
			if(get_tick() - order_start_poll_tick >= ORDER_PERIOD/TIME_PER_TICK)
			{
				add_order_package(2);
			}
			else if(get_order_result())
			{
				add_order_package(get_order_result());
			}
		}
		else
		{
			if(get_tick() >= (ORDER_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - order_start_poll_tick)))
			{
				add_order_package(2);
			}
			else if(get_order_result())
			{
				add_order_package(get_order_result());
			}
		}
	}
}

void temp_upload_poll(void)
{
	static unsigned int last_temp_tick = 0;

	if(0xFFFFFFFF - last_temp_tick >= TEMP_PERIOD/TIME_PER_TICK)
	{
		if(get_tick() - last_temp_tick >= TEMP_PERIOD/TIME_PER_TICK)
		{
			add_send_package(0, 1, 0);
			change_display_to(TEMP_STATUS, 0xFF, 0);
			last_temp_tick = get_tick();
		}
	}else
	{
		if(get_tick() > (TEMP_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - last_temp_tick)))
		{
			add_send_package(0, 1, 0);
			change_display_to(TEMP_STATUS, 0xFF, 0);
			last_temp_tick = get_tick();
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
		if(!parse_js((char *)USART2_RX_BUF));
		{
			process_server_cmd();
		}
		USART2_RX_STA=0;
	}
}

void display_poll(void)
{
	is_display_status_need_change();
}

void bt_send_poll(void)
{
	static unsigned int last_send_tick = 0;

	if(0xFFFFFFFF - last_send_tick >= SEND_PERIOD/TIME_PER_TICK)
	{
		if(get_tick() - last_send_tick >= SEND_PERIOD/TIME_PER_TICK)
		{
			if(get_pending_package_num() != 0)
			{
				send_max_priority_package();
				last_send_tick = get_tick();
			}
		}
	}else
	{
		if(get_tick() > (SEND_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - last_send_tick)))
		{
			if(get_pending_package_num() != 0)
			{
				send_max_priority_package();
				last_send_tick = get_tick();
			}
		}
	}
}
