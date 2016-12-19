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
#include "system.h"

unsigned int tick;

int main(void)
{
	unsigned char order_response = 0, i, ret = 1;

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
		HC05_connect_check();

		if(accident_sta_get())
		{
			bt_resp_send(0, 0, 1);
			accident_sta_reset();
		}
		if(!order_resp_poll(&order_response))
		{
			bt_resp_send(order_response, 1, 0);
		}

		temp_upload_poll();
		bt_receive();
		dir_display_poll();
		tick++;
		delay_ms(TICK_PERIOD);
	}

}
