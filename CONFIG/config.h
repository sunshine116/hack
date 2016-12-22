#ifndef __CONFIG_H_
#define __CONFIG_H_

#define ACCIDENT_PERIOD 			5000 //unit is ms
#define ORDER_PERIOD 				5000 //unit is ms
#define DIR_PERIOD					5000 //unit is ms
#define TEMP_PERIOD 				5000 //unit is ms
#define SEND_PERIOD 				2000 //unit is ms

#define TICK_PERIOD 				50 //unit is ms

#define SYSTEM_TICK 				1000 //unit is ms
#define TIME_PER_TICK 				(1000/SYSTEM_TICK) //unit is ms

#define MAX_PENDING_PACKAGE_NUM   	10

#endif
