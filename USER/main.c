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

int main(void)
{
	unsigned int i = 0;

	NVIC_Configuration();
	delay_init();
	uart_init(9600);
	LED_Init();
	KEY_Init();
	OLED_Init();

	while(HC05_Init())
	{
		OLED_print_error("BT init error!");
		delay_ms(1000);
	}
	OLED_Clear();

	while(HC05_Get_Role() != 0)
	{
		OLED_print_error("BT role set error!");
		HC05_Set_Cmd("AT+ROLE=0");
		HC05_Set_Cmd("AT+RESET");
		delay_ms(1000);
	}
	OLED_Clear();

	while(1)
	{
		HC05_connect_check();
		if(0 == i%100)
		{
			u2_printf("{\"UID\":\"hello001\",\"MID\":\"110\",\"data\":{\"dir\":%s,\"query\":\"order\"}}\r\n", "hello");
		}

		if(0 == i%100)
		{
			OLED_display(3, 255);
		}
		bt_receive();
		i++;
		delay_ms(50);
	}

}
