#ifndef __DISPLAY_H_
#define __DISPLAY_H_

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
enum OLED_CONTENT
{
	BT_DISCONN = 0,
	BT_CONNECT,
	TURN_AROUND,
	LEFT,
	RIGHT,
	FORWARD,
	ORDER,
	TEMP
};

enum DISPLAY_STATUS
{
	BT_STATUS = 0,
	ORDER_STATUS,
	DIR_STATUS,
	TEMP_STATUS
};
void OLED_display_error(u8 *string);
void is_display_status_need_change(void);
void change_display_to(unsigned char status, unsigned char arg, unsigned char force_flag);

#endif
