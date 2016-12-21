#include <string.h>
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "usart.h"
#include "usart2.h"
#include "hc05.h"
#include "key.h"
#include "led.h"
#include "DS18B20.h"
#include "js_parse.h"
#include "exti.h"
#include "DS18B20.h"
#include "poll.h"
#include "bt_send.h"

int main(void)
{
	unsigned char i, ret = 1;

	Stm32_Clock_Init(9);
	NVIC_Configuration();
	uart_init(9600);
	ret = system_tick_init();
	if(ret != 0)
	{
		printf("system tick init error!\r\n");
	}
	LED_Init();
	KEY_Init();
	OLED_Init();
	EXTI_Init();
	ReadTemperature();
	init_send_package();

	while(HC05_Init())
	{
		OLED_print_error("BT init error!");
		for(i = 0; i < 10; i++)
			delay_ms(100);
	}
	OLED_Clear();

	while(HC05_Get_Role() != 0)
	{
		OLED_print_error("BT role set error!");
		HC05_Set_Cmd("AT+ROLE=0");
		HC05_Set_Cmd("AT+RESET");
		for(i = 0; i < 10; i++)
			delay_ms(100);
	}
	OLED_Clear();

	while(1)
	{
		HC05_connect_poll();
		accident_sta_poll();
		order_resp_poll();
		temp_upload_poll();
		bt_receive_poll();
		bt_send_poll();
		dir_display_poll();
	}

}
