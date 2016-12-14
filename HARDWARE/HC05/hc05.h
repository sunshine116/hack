#ifndef __HC05_H
#define __HC05_H	 
#include "sys.h" 

#define HC05_KEY  	PBout(10) 		//蓝牙控制KEY信号
#define HC05_LED  	PBin(11)		//蓝牙连接状态信号
  
u8 HC05_Init(void);
void HC05_CFG_CMD(u8 *str);
u8 HC05_Get_Role(void);
u8 HC05_Set_Cmd(u8* atstr);
void HC05_connect_check(void);
void HC05_Role_Show(void);
#endif  
















