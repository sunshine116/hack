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

int main(void)
{
	unsigned char reclen=0;
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
		if(0 == i%20)
		{
			u2_printf("Hello pretty\r\n");
		}

		if(0 == i%100)
		{
			OLED_display(3, 255);
		}
		if(USART2_RX_STA&0X8000)
		{
			printf("Receive: ");
 			reclen=USART2_RX_STA&0X7FFF;
			USART2_RX_BUF[reclen]=0;
			printf("%s\r\n",USART2_RX_BUF);
			if(strcmp((const char *)USART2_RX_BUF, "right") == 0)
				OLED_display(1, 3);
			if(strcmp((const char *)USART2_RX_BUF, "left") == 0)
				OLED_display(1, 2);
			if(strcmp((const char *)USART2_RX_BUF, "forward") == 0)
				OLED_display(1, 0);
			if(strcmp((const char *)USART2_RX_BUF, "turn round") == 0)
				OLED_display(1, 1);
			USART2_RX_STA=0;
			i = 0;
		}
		i++;
		delay_ms(50);
	}

}
