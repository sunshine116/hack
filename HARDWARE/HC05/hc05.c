#include "delay.h" 			 
#include "usart.h" 			 
#include "usart2.h" 			 
#include "hc05.h" 
#include "led.h" 
#include "string.h"	 
#include "math.h"
#include "oled.h"

//初始化ATK-HC05模块
//返回值:0,成功;1,失败.
u8 HC05_Init(void)
{
	u8 retry=10,t;	  		 
	u8 temp=1;
	RCC->APB2ENR|=1<<3;    	//使能PORTB时钟
	GPIOB->CRH&=0XFFFF0FFF;	//PB11,输入
	GPIOB->CRH|=0X00008000;
	GPIOB->ODR|=1<<11; 		//PB11上拉
	GPIOB->CRH&=0XFFFFF0FF;	//PB10,推挽输出
	GPIOB->CRH|=0X00000300;
	GPIOB->ODR|=1<<10; 		//PB10输出1
	USART2_Init(36,9600);	//初始化串口2为:9600,波特率.
	while(retry--)
	{
		HC05_KEY=1;					//KEY置高,进入AT模式
		delay_ms(10);
		u2_printf("AT\r\n");		//发送AT测试指令
		HC05_KEY=0;					//KEY拉低,退出AT模式
		for(t=0;t<10;t++) 			//最长等待50ms,来接收HC05模块的回应
		{
			if(USART2_RX_STA&0X8000)break;
			delay_ms(5);
		}		
		if(USART2_RX_STA&0X8000)	//接收到一次数据了
		{
			temp=USART2_RX_STA&0X7FFF;	//得到数据长度
			USART2_RX_STA=0;			 
			if(temp==4&&USART2_RX_BUF[0]=='O'&&USART2_RX_BUF[1]=='K')
			{
				temp=0;//接收到OK响应
				break;
			}
		}
	}		    
	temp += HC05_Set_Cmd("AT+NAME=MTMT");
	temp += HC05_Set_Cmd("AT+PSWD=meituan");
	if(retry==0 || temp != 0)temp=1;	//检测失败
	return temp;	 
}	 
//获取ATK-HC05模块的角色
//返回值:0,从机;1,主机;0XFF,获取失败.							  
u8 HC05_Get_Role(void)
{	 		    
	u8 retry=0X0F;
	u8 temp,t;
	while(retry--)
	{
		HC05_KEY=1;					//KEY置高,进入AT模式
		delay_ms(10);
		u2_printf("AT+ROLE?\r\n");	//查询角色
		for(t=0;t<20;t++) 			//最长等待200ms,来接收HC05模块的回应
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)break;
		}		
		HC05_KEY=0;					//KEY拉低,退出AT模式
		if(USART2_RX_STA&0X8000)	//接收到一次数据了
		{
			temp=USART2_RX_STA&0X7FFF;	//得到数据长度
			USART2_RX_STA=0;			 
			if(temp==13&&USART2_RX_BUF[0]=='+')//接收到正确的应答了
			{
				temp=USART2_RX_BUF[6]-'0';//得到主从模式值
				break;
			}
		}		
	}
	if(retry==0)temp=0XFF;//查询失败.
	return temp;
} 							   
//ATK-HC05设置命令
//此函数用于设置ATK-HC05,适用于仅返回OK应答的AT指令
//atstr:AT指令串.比如:"AT+RESET"/"AT+UART=9600,0,0"/"AT+ROLE=0"等字符串
//返回值:0,设置成功;其他,设置失败.							  
u8 HC05_Set_Cmd(u8* atstr)
{	 		    
	u8 retry=0X0F;
	u8 temp,t;
	while(retry--)
	{
		HC05_KEY=1;					//KEY置高,进入AT模式
		delay_ms(10);
		u2_printf("%s\r\n",atstr);	//发送AT字符串
		HC05_KEY=0;					//KEY拉低,退出AT模式
		for(t=0;t<20;t++) 			//最长等待100ms,来接收HC05模块的回应
		{
			if(USART2_RX_STA&0X8000)break;
			delay_ms(5);
		}		
		if(USART2_RX_STA&0X8000)	//接收到一次数据了
		{
			temp=USART2_RX_STA&0X7FFF;	//得到数据长度
			USART2_RX_STA=0;			 
			if(temp==4&&USART2_RX_BUF[0]=='O')//接收到正确的应答了
			{			
				temp=0;
				break;			 
			}
		}		
	}
	if(retry==0)temp=0XFF;//设置失败.
	return temp;
} 
///////////////////////////////////////////////////////////////////////////////////////////////////
//通过该函数,可以利用USMART,调试接在串口2上的ATK-HC05模块
//str:命令串.(这里注意不再需要再输入回车符)
void HC05_CFG_CMD(u8 *str)
{					  
	u8 temp;
	u8 t;		  
	HC05_KEY=1;						//KEY置高,进入AT模式
	delay_ms(10);
	u2_printf("%s\r\n",(char*)str); //发送指令
	for(t=0;t<50;t++) 				//最长等待500ms,来接收HC05模块的回应
	{
		if(USART2_RX_STA&0X8000)break;
		delay_ms(10);
	}									    
	HC05_KEY=0;						//KEY拉低,退出AT模式
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{
		temp=USART2_RX_STA&0X7FFF;	//得到数据长度
		USART2_RX_STA=0;
		USART2_RX_BUF[temp]=0;		//加结束符		 
	} 				 
}

//显示ATK-HC05模块的连接状态
void HC05_connect_check(void)
{
	static unsigned char flag = 0;
	unsigned char i;
	if(HC05_LED)
	{
		if(0 == flag)
		{
			OLED_display(2, 5);
			flag = 1;
			for(i = 0; i < 30; i++)
				delay_ms(100);
		}
	}
	else
	{
		flag = 0;
		OLED_display(0, 4);
		while(!HC05_LED)
		{
			for(i = 0; i < 10; i++)
				delay_ms(100);
		}
	}
}

//显示ATK-HC05模块的主从状态
void HC05_Role_Show(void)
{
	if(HC05_Get_Role()==1)printf("ROLE:Master");	//主机
	else printf("ROLE:Slave ");			 		//从机
}
