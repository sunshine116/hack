#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "bt_send.h"
#include "oled.h"
#include "poll.h"
#include "delay.h"
#include "DS18B20.h"
#include "usart2.h"
#include "js_parse.h"

extern unsigned char order_display_flag;

static bt_package_t *PACKAGE_HEAD = NULL;
static bt_package_t *PACKAGE_TAIL = NULL;

void bt_send_order(unsigned char order_response)
{
	js_compose(order_response, 1, 0);
	stop_order_poll();
}

unsigned char bt_send(void)
{
	u2_printf("%s", PACKAGE_HEAD->json);
	return 0;
}

unsigned char add_package(char *json, unsigned char type)
{
	bt_package_t *package_tmp;

	package_tmp = (bt_package_t *)malloc(sizeof(bt_package_t));
	if(package_tmp == NULL)
	{
		printf("package_tmp malloc error!\r\n");
		return 1;
	}
	if(PACKAGE_HEAD == NULL)
	{
		PACKAGE_HEAD = package_tmp;
		PACKAGE_TAIL = package_tmp;
		PACKAGE_HEAD->json = json;
		PACKAGE_HEAD->type= type;
	}
	else
	{
		package_tmp->json = json;
		package_tmp->type= type;
		PACKAGE_TAIL->next = package_tmp;
		PACKAGE_TAIL = package_tmp;
	}
	return 0;
}

void delete_package(void)
{
	bt_package_t *tmp;

	if(PACKAGE_HEAD == PACKAGE_TAIL)
	{
		if(PACKAGE_TAIL == NULL)
		{
			return;
		}
		free(PACKAGE_HEAD->json);
		free(PACKAGE_HEAD);
		PACKAGE_HEAD = NULL;
		PACKAGE_TAIL = NULL;
	}
	else
	{
		tmp = PACKAGE_HEAD;
		PACKAGE_HEAD = tmp->next;
		free(tmp->json);
		free(tmp);
	}
}

bt_package_t *get_package_head(void)
{
	return PACKAGE_HEAD;
}
