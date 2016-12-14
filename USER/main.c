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
	unsigned char i = 0, reclen=0;
	unsigned char symbol, dot;
	unsigned int integer;

	NVIC_Configuration();
	delay_init();
	uart_init(9600);
	LED_Init();
	KEY_Init();
	OLED_Init();

	ReadTemperature(&symbol, &integer, &dot);
	printf("symbol: %d  integer: %d   dot: %d\r\n", symbol, integer, dot);

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
		if(i == 20)
		{
			u2_printf("Hello pretty\r\n");
			i = 0;
		}
		if(USART2_RX_STA&0X8000)
		{
			printf("Receive: ");
 			reclen=USART2_RX_STA&0X7FFF;
			USART2_RX_BUF[reclen]=0;
			printf("%s\r\n",USART2_RX_BUF);
			USART2_RX_STA=0;
		}
		i++;
		delay_ms(50);
	}

}
