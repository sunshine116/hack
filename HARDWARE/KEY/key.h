#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY_PRES	0		//KEY按下
#define KEY_UP  	2		//KEY按下

#define KEY0  	PBin(6)
#define KEY1  	PAin(0)
#define KEY2 	PCin(13)
	 
void KEY_Init(void);		//IO初始化
u8 KEY_Scan(u8 mode);		//按键扫描函数
#endif
