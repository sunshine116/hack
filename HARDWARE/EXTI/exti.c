#include <stdio.h>
#include "sys.h"
#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "DS18B20.h"
#include "oled.h"

extern unsigned char order_display_flag;

static unsigned int accident_tick = 0;
static unsigned char order_flag = 0xFF;
static unsigned char order_poll_flag = 0xFF;
static unsigned int order_start_tick = 0;
static unsigned char accident_flag = 0;
//外部中断0服务程序
void EXTI0_IRQHandler(void)
{
	delay_ms_int(10);
	if(KEY1==0)
	{
		accident_flag = 1;
	}		 
	EXTI->PR=1<<0;
}

void EXTI9_5_IRQHandler(void)
{			
	delay_ms_int(10);
    if(KEY0==0)
	{
		order_flag = 2; //no
	}
	EXTI->PR=1<<6;
}

void EXTI15_10_IRQHandler(void)
{
	delay_ms_int(10);
    if(KEY2==1)
	{
		order_flag = 1;  //yes
	}
	EXTI->PR=1<<13;
}

void EXTI_Init(void)
{
	KEY_Init();
	Ex_NVIC_Config(GPIO_A,0,FTIR);
	Ex_NVIC_Config(GPIO_B,6,FTIR);
	Ex_NVIC_Config(GPIO_C,13,RTIR);

	MY_NVIC_Init(2,2,EXTI0_IRQn,2);
	MY_NVIC_Init(2,1,EXTI9_5_IRQn,2);
	MY_NVIC_Init(2,0,EXTI15_10_IRQn,2);
}

void order_resp_start(void)
{
	order_start_tick = get_tick();
	order_flag = 0;
	order_poll_flag = 0;
	order_display_flag = 1;
	OLED_display(4, 6);
}
static unsigned char reset_order_flags(void)
{
	order_poll_flag = 0xFF;
	return 0;
}

unsigned char accident_sta_get(void)
{
	if(accident_flag)
	{
		if(0xFFFFFFFF - accident_tick >= ACCIDENT_PERIOD/TIME_PER_TICK)
		{
			if(get_tick() - accident_tick >= ACCIDENT_PERIOD/TIME_PER_TICK)
			{
				accident_tick = get_tick();
				accident_flag = 0;
				return 1;
			}
		}
		else
		{
			if(get_tick() >= (ACCIDENT_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - accident_tick)))
			{
				accident_tick = get_tick();
				accident_flag = 0;
				return 1;
			}
		}
	}
	return 0;
}
//0: read the tmp
//1: poll contiune
unsigned char order_resp_poll(unsigned char *tmp)
{
	if(order_poll_flag == 0)
	{
		if(0xFFFFFFFF - order_start_tick >= ORDER_PERIOD/TIME_PER_TICK)
		{
			if(get_tick() - order_start_tick >= ORDER_PERIOD/TIME_PER_TICK)
			{
				*tmp = 2;
				return reset_order_flags();
			}
			else if(order_flag)
			{
				*tmp = order_flag;
				return reset_order_flags();
			}
		}
		else
		{
			if(get_tick() >= (ORDER_PERIOD/TIME_PER_TICK - (0xFFFFFFFF - accident_tick)))
			{
				*tmp = 2;
				return reset_order_flags();
			}
			else if(order_flag)
			{
				*tmp = order_flag;
				return reset_order_flags();
			}
		}
	}
	return 1;
}
