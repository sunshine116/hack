#ifndef __EXTI_H
#define __EXIT_H	 
#include "sys.h"

void EXTI_Init(void);	//外部中断初始化
unsigned char is_accident_happen(void);
unsigned char get_order_result(void);
void set_order_flag(unsigned char tmp);
void clear_accident_flag(void);

#endif

























