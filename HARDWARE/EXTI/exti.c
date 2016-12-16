#include <stdio.h>
#include "sys.h"
#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "DS18B20.h"

extern unsigned int tick;
static unsigned char order_flag = 0xFF;
static unsigned char order_poll_flag = 0xFF;
static unsigned int order_start_tick = 0;
//外部中断0服务程序
void EXTI0_IRQHandler(void)
{
	delay_ms(20);	//消抖
	if(KEY1==0)	//WK_UP按键
	{
		printf("KEY0\r\n");
	}		 
	EXTI->PR=1<<0;  //清除LINE0上的中断标志位  
}
//外部中断9~5服务程序
void EXTI9_5_IRQHandler(void)
{			
	delay_ms(20);   //消抖			 
    if(KEY0==0)		//按键0
	{
		printf("yes\r\n");
		order_flag = 1; //yes
	}
 	EXTI->PR=1<<6;     //清除LINE6上的中断标志位  
}
//外部中断15~10服务程序
void EXTI15_10_IRQHandler(void)
{			
	delay_ms(20);   //消抖			 
    if(KEY2==1)		//按键1
	{
		printf("no\r\n");
		order_flag = 2;  //no
	}
 	EXTI->PR=1<<13; //清除LINE13上的中断标志位  
}
//外部中断初始化程序
//初始化PB6 PB7 PB8为中断输入.
void EXTI_Init(void)
{
	KEY_Init();
	Ex_NVIC_Config(GPIO_A,0,FTIR); 		//下降沿触发
	Ex_NVIC_Config(GPIO_B,6,FTIR);		//下降沿触发
	Ex_NVIC_Config(GPIO_C,13,RTIR);		//上降沿触发

	MY_NVIC_Init(2,2,EXTI0_IRQn,2);    	//抢占2，子优先级2，组2
	MY_NVIC_Init(2,1,EXTI9_5_IRQn,2);  	//抢占2，子优先级1，组2
	MY_NVIC_Init(2,0,EXTI15_10_IRQn,2);	//抢占2，子优先级0，组2	   
}

void order_resp_start(void)
{
	order_start_tick = tick;
	order_flag = 0;
	order_poll_flag = 0;
}

//0: read the tmp
//1: poll contiune
unsigned char order_resp_poll(unsigned char *tmp)
{
	if(order_poll_flag == 0)
	{
		if(tick - order_start_tick < POLL_PERIOD/TICK_PERIOD)
		{
			if(order_flag)
			{
				*tmp = order_flag;
				temp_tick_set(tick);
				order_poll_flag = 0xFF;
				return 0;
			}
		}
		else
		{
			*tmp = 2;
			temp_tick_set(tick);
			order_poll_flag = 0xFF;
			return 0;
		}
		return 1;
	}
}


