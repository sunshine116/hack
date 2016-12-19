#include "delay.h"
#include "sys.h"

void delay_us(u32 nus)
{
	u16 i;

	while(nus--)
	{
		i=6;
		while(i--);
	}
}

void delay_ms(u16 nms)
{
	unsigned int start_tick;

	start_tick = get_tick();
	if(start_tick + nms/TIME_PER_TICK <= 0xFFFFFFFF)
	{
		while(get_tick() - start_tick < nms/TIME_PER_TICK);
	}
	else
	{
		while(get_tick() >= nms/TIME_PER_TICK - (0xFFFFFFFF - start_tick));
	}
}

void delay_ms_int(u16 nms)
{
	u16 i;

	while(nms--)
	{
		i=7200;
		while(i--);
	}
}
