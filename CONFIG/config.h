#ifndef __CONFIG_H_
#define __CONFIG_H_

#define VERSION						"0.3"
//poll period
#define ACCIDENT_PERIOD 			5000 //unit is ms
#define ORDER_PERIOD 				5000 //unit is ms
#define DIR_PERIOD					5000 //unit is ms
#define TEMP_PERIOD 				5000 //unit is ms
#define SEND_PERIOD 				2000 //unit is ms

//sys tick config
#define SYSTEM_TICK 				1000 //unit is ms
#define TIME_PER_TICK 				(1000/SYSTEM_TICK) //unit is ms

//package config
#define MAX_PENDING_PACKAGE_NUM   	10

//display config
#define MAX_DISPLAY_TIME            TEMP_DISPLAY_TIME //change to the max of display time
#define BT_CONN_DISPLAY_TIME		3000 //unit is ms
#define ORDER_DISPLAY_TIME			5000 //unit is ms
#define DIR_DISPLAY_TIME			3000 //unit is ms
#define TEMP_DISPLAY_TIME			10000 //unit is ms

#endif
