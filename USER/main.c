#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "bmp.h"
#include "usart.h"
#include "usart2.h"
#include "hc05.h"
#include "key.h"
#include "led.h"

//显示ATK-HC05模块的主从状态
void HC05_Role_Show(void)
{
	if(HC05_Get_Role()==1)printf("ROLE:Master");	//主机
	else printf("ROLE:Slave ");			 		//从机
}
//显示ATK-HC05模块的连接状态
void HC05_Sta_Show(void)
{												 
	if(HC05_LED)printf("STA:Connected ");			//连接成功
	else printf("STA:Disconnect");	 			//未连接				 
}
int main(void)
{	 
	u8 t;
	u8 key;
	u8 sendmask=0;
	u8 sendcnt=0;
	u8 sendbuf[20];	  
	u8 reclen=0;  
   	// Stm32_Clock_Init(9);	//系统时钟设置
	delay_init();			//延时初始化
	uart_init(9600); 	    //串口1初始化为9600
	LED_Init();				//初始化与LED连接的硬件接口
	KEY_Init();				//初始化按键
	printf("ALIENTEK STM32 ^_^");	
	printf("HC05 BLUETOOTH COM TEST");	
	printf("ATOM@ALIENTEK");
	while(HC05_Init()) 		//初始化ATK-HC05模块  
	{
		printf("ATK-HC05 Error!"); 
		delay_ms(500);
		printf("Please Check!!!"); 
		delay_ms(100);
	}	 										   	   
	printf("WK_UP:ROLE KEY0:SEND/STOP");  
	printf("ATK-HC05 Standby!");  
  	printf("Send:");	
	printf("Receive:");	

	HC05_Role_Show();	  
 	while(1) 
	{		
		key=KEY_Scan(0);
		if(key==WKUP_PRES)							//切换模块主从设置
		{
   			key=HC05_Get_Role();
			if(key!=0XFF)
			{
				key=!key;  					//状态取反	   
				if(key==0)HC05_Set_Cmd("AT+ROLE=0");
				else HC05_Set_Cmd("AT+ROLE=1");
				HC05_Role_Show();
				HC05_Set_Cmd("AT+RESET");	//复位ATK-HC05模块
			}
		}else if(key==KEY0_PRES)
		{
			sendmask=!sendmask;				//发送/停止发送  	 
			if(sendmask==0)printf("WHITE");//清除显示
		}else delay_ms(10);	   
		if(t==50)
		{
			if(sendmask)					//定时发送
			{
				sprintf((char*)sendbuf,"ALIENTEK HC05 %d\r\n",sendcnt);
	  			printf("%s\n",sendbuf);	//显示发送数据	
				u2_printf("ALIENTEK HC05 %d\r\n",sendcnt);		//发送到蓝牙模块
				sendcnt++;
				if(sendcnt>99)sendcnt=0;
			}
			HC05_Sta_Show();  	  
			t=0;
			LED0=!LED0; 	     
		}	  
		if(USART2_RX_STA&0X8000)			//接收到一次数据了
		{
			printf("WHITE");	//清除显示
 			reclen=USART2_RX_STA&0X7FFF;	//得到数据长度
		  	USART2_RX_BUF[reclen]=0;	 	//加入结束符
			if(reclen==9||reclen==8) 		//控制DS1检测
			{
				if(strcmp((const char*)USART2_RX_BUF,"+LED1 ON")==0)LED1=0;	//打开LED1
				if(strcmp((const char*)USART2_RX_BUF,"+LED1 OFF")==0)LED1=1;//关闭LED1
			}
 			printf("%s\n",USART2_RX_BUF);//显示接收到的数据
 			USART2_RX_STA=0;	 
		}	 															     				   
		t++;	
	}											    
}
