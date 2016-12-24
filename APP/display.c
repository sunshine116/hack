#include <stdio.h>
#include "sys.h"
#include "delay.h"
#include "poll.h"
#include "display.h"
#include "oled.h"
#include "DS18B20.h"

static unsigned char DISPLAY_STATUS_CURRENT = TEMP_STATUS;//0: 蓝牙连接成功 1: 是否接单 2: 导航 3: 温度

static unsigned int bt_connect_display_last_tick = 0;
static unsigned int order_display_last_tick = 0;
static unsigned int dir_display_last_tick = 0;
static unsigned int temp_display_last_tick = 0;

/**
 * OLED title display
 * @param title 	0: 		注意！
 *                  1: 		导航模式
 *                  2: 		蓝牙状态
 *                  3: 		测温模式
 *                  4: 		接单模式
 */
static unsigned char mod[][6] = {     // first num: num of words   second num: x start pos  remain: words location
	{3, 41, 0, 1, 2, 0},
	{4, 25, 3, 4, 5, 6},
	{4, 25, 7, 8, 9, 10},
	{4, 25, 11, 12, 5, 6},
	{4, 25, 13, 14, 5, 6},
};
static void OLED_title_display(unsigned char title)
{
	unsigned char i;

	if(title > (sizeof(mod)/sizeof(mod[0])))
		OLED_display_error("CH16 error");
	for(i =0; i < mod[title][0]; i++)
	{
		OLED_ShowCHinese_16(mod[title][1] + 16*i, 0, mod[title][2 + i]);
	}
}

/**
 * OLED content display
 * @param content 	0: 前方直行
 *                  1: 前方掉头
 *                  2: 前方左转
 *                  3: 前方右转
 *                  4: 蓝牙断开
 *                  5: 连接成功
 *                  6: 是否接单
 */
static unsigned char dir[][4] = {
	{3, 4, 7, 8},
	{3, 4, 5, 6},
	{3, 4, 0, 2},
	{3, 4, 1, 2},
	{9, 10, 11, 12},
	{13, 14, 15, 16},
	{17, 18, 19, 20},
};
static void OLED_content_display(unsigned char content)
{
	unsigned char i;

	if(content > (sizeof(dir)/sizeof(dir[0])))
		OLED_display_error("CH32 error");
	for(i =0; i < 4; i++)
	{
		OLED_ShowCHinese_32(i*32, 3, dir[content][i]);
	}
}

/**
 * OLED title display
 * @param title 	0: 		注意！
 *                  1: 		导航模式
 *                  2: 		蓝牙状态
 *                  3: 		测温模式
 *                  4: 		接单模式
 */
/**
 * OLED content display
 * @param content 	255: 	显示温度
 * 	               	0: 		前方直行
 *                  1: 		前方掉头
 *                  2: 		前方左转
 *                  3: 		前方右转
 *                  4: 		蓝牙断开
 *                  5: 		连接成功
 *                  6: 		是否接单
 */
static void OLED_display_content(unsigned char title, unsigned char content)
{
	unsigned char symbol, dot, x_pos = 5, i = 0;
	unsigned int integer;

	OLED_Clear();
	OLED_title_display(title);
	if(255 == content)
	{
		Temp_get(&symbol, &integer, &dot);
		OLED_ShowNumber_16X32(x_pos, 3, 11 + symbol); i++;
		if(integer/10 != 0)
		{
			OLED_ShowNumber_16X32((x_pos+16*i), 3, integer/10); i++;
		}
		OLED_ShowNumber_16X32((x_pos+16*i), 3, integer%10); i++;
		OLED_ShowNumber_16X32((x_pos+16*i), 3, 10); i++;
		if(dot/10 != 0)
		{
			OLED_ShowNumber_16X32((x_pos+16*i), 3, dot/10); i++;
		}
		OLED_ShowNumber_16X32((x_pos+16*i), 3, dot%10); i++;
		OLED_ShowNumber_16X32((x_pos+16*i), 3, 13);
	}
	else
	{
		OLED_content_display(content);
	}
}

/**
 * OLED status display
 * @param status 	0: 	 	蓝牙断开
 * 	               	1: 		连接成功
 *                  2: 		前方掉头
 *                  3: 		前方左转
 *                  4: 		前方右转
 *                  5: 		前方直行
 *                  6: 		是否接单
 *                  7: 		显示温度
 */
static void OLED_display_status(unsigned char status)
{
	switch(status)
	{
		case BT_DISCONN:
			OLED_display_content(0, 4);
			break;
		case BT_CONNECT:
			OLED_display_content(2, 5);
			break;
		case TURN_AROUND:
			OLED_display_content(1, 1);
			break;
		case LEFT:
			OLED_display_content(1, 2);
			break;
		case RIGHT:
			OLED_display_content(1, 3);
			break;
		case FORWARD:
			OLED_display_content(1, 0);
			break;
		case ORDER:
			OLED_display_content(4, 6);
			break;
		case TEMP:
			OLED_display_content(3, 255);
			break;
	}
}

//0: time is out 1: time is not out
static unsigned char is_display_timeout(unsigned int tick_now, unsigned int last_tick)
{
	if(tick_now > last_tick)
	{
		if(tick_now - last_tick > MAX_DISPLAY_TIME/TIME_PER_TICK)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if(tick_now < last_tick)
	{
		if(last_tick - tick_now > MAX_DISPLAY_TIME/TIME_PER_TICK)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 0;
	}

}

static void change_status_if_timeout(unsigned int last_tick)
{
	unsigned int tick = get_tick();
	unsigned char ret = 1;

	ret = is_display_timeout(tick, last_tick);
	if(ret == 0)
	{
		change_display_to(TEMP_STATUS, 0xFF, 1);
	}
}

void OLED_display_error(u8 *string)
{
	OLED_Clear();
	OLED_title_display(0);
	OLED_ShowString(0,2,string);
}

void change_display_to(unsigned char status, unsigned char arg, unsigned char force_flag)
{
	static unsigned char DISPLAY_CURRENT_ARG = 0xFF;

	if(force_flag != 1)
	{
		if(status > DISPLAY_STATUS_CURRENT)
		{
			return;
		}
		else if(status == DISPLAY_STATUS_CURRENT)
		{
			if(arg == DISPLAY_CURRENT_ARG)
			{
				return;
			}
		}
	}
	DISPLAY_CURRENT_ARG = arg;
	DISPLAY_STATUS_CURRENT = status;
	switch(status)
	{
		case BT_STATUS:
		    OLED_display_status(arg);
			bt_connect_display_last_tick = get_tick() + BT_CONN_DISPLAY_TIME/TIME_PER_TICK;
			break;
		case ORDER_STATUS:
		    OLED_display_status(ORDER);
			order_display_last_tick = get_tick() + ORDER_DISPLAY_TIME/TIME_PER_TICK;
			break;
		case DIR_STATUS:
		    OLED_display_status(arg);
			dir_display_last_tick = get_tick() + DIR_DISPLAY_TIME/TIME_PER_TICK;
			break;
		case TEMP_STATUS:
		    OLED_display_status(TEMP);
			temp_display_last_tick = get_tick() + TEMP_DISPLAY_TIME/TIME_PER_TICK;
			break;
		default:
			printf("change status error!\r\n");
	}
}

void is_display_status_need_change(void)
{
	switch(DISPLAY_STATUS_CURRENT)
	{
		case BT_STATUS:
						change_status_if_timeout(bt_connect_display_last_tick);
						break;
		case ORDER_STATUS:
						change_status_if_timeout(order_display_last_tick);
						break;
		case DIR_STATUS:
						change_status_if_timeout(dir_display_last_tick);
						break;
		default:
						change_status_if_timeout(temp_display_last_tick);
	}
}

