#include <stdio.h>
#include "sys.h"
#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "DS18B20.h"
#include "oled.h"

static unsigned char accident_flag = 0;
static unsigned char order_flag = 0;

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

unsigned char is_accident_happen(void)
{
	return accident_flag;
}

void clear_accident_flag(void)
{
	accident_flag = 0;
}

unsigned char get_order_result(void)
{
	return order_flag;
}

void set_order_flag(unsigned char tmp)
{
	order_flag = tmp;
}
