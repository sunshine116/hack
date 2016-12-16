#ifndef __EXTI_H
#define __EXIT_H	 
#include "sys.h"

unsigned char order_resp_poll(unsigned char *tmp);
void order_resp_start(void);
void EXTI_Init(void);	//外部中断初始化
unsigned char accident_sta_get(void);
void accident_sta_reset(void);

#endif

























