#include "key.h"
#include "delay.h"
#include "sys.h"

//按键初始化函数 
//PA0.15和PC5 设置成输入
void KEY_Init(void)
{
	RCC->APB2ENR|=1<<2;     //使能PORTA时钟
	RCC->APB2ENR|=1<<3;     //使能PORTB时钟
	RCC->APB2ENR|=1<<4;     //使能PORTC时钟
	JTAG_Set(SWD_ENABLE);	//关闭JTAG,开启SWD
	GPIOA->CRL&=0XFFFFFFF0;	//PA0设置成输入
	GPIOA->CRL|=0X00000008;
	GPIOA->ODR|=1<<0;	   	//PA0上拉

	GPIOB->CRL&=0XF0FFFFFF;	//PB6 PB7设置成输入
	GPIOB->CRL|=0X08000000;
	GPIOB->ODR|=1<<6;	   	//PB8上拉

	GPIOC->CRH&=0XFF0FFFFF;	//PC13设置成输入
	GPIOC->CRH|=0X00800000;
}
