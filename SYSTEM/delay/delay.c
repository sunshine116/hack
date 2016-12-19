#include "delay.h"
#include "sys.h"
#include "system.h"

void delay_us(u32 nus)
{
	unsigned char i = 0;
	while(nus)
	{
		i = 5;
		while(i)
			i--;
		nus--;
	}
}

void delay_ms(u16 nms)
{
	unsigned int start_tick;

	start_tick = get_tick();
	if(start_tick + nms/(1000/SYSTEM_TICK)<= 0xFFFFFFFF)
	{
		while(get_tick() - start_tick < nms/(1000/SYSTEM_TICK));
	}
	else
	{
		while(get_tick() >= nms/(1000/SYSTEM_TICK) - (0xFFFFFFFF - start_tick));
	}
}
