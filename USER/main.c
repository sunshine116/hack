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

unsigned int tick;

int main(void)
{
	unsigned char order_response = 0;

	NVIC_Configuration();
	delay_init();
	uart_init(9600);
	LED_Init();
	KEY_Init();
	OLED_Init();
	EXTI_Init();
	ReadTemperature();

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

		if(!order_resp_poll(&order_response))
		{
			bt_resp_send(order_response, 1);
		}

		temp_upload_poll();
		bt_receive();
		tick++;
		delay_ms(TICK_PERIOD);
	}

}
