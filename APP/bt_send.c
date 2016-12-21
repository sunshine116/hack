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
static unsigned int PENDING_PACKAGE = 0;

static bt_package_t TEMP_HEAD = {NULL, 0xFF, NULL};
static bt_package_t ORDER_HEAD = {NULL, 0xFF, &TEMP_HEAD};
static bt_package_t ACCIDENT_HEAD = {NULL, 0xFF, &ORDER_HEAD};

void add_order_package(unsigned char order_response)
{
	printf("!!!!!add_order_package!!!!!!\r\n");
	add_send_package(order_response, 1, 0);
	stop_order_poll();
	reset_orderId();
}

void bt_send(void)
{
	bt_package_t *head = &ACCIDENT_HEAD;

	while(head->type == 0xFF)
	{
		head = head->next;
		if(head == NULL)
		{
			return;
		}
	}
	u2_printf("%s", head->json);
	delete_send_package_head();
}

static void add_to_tail(bt_package_t *old_tail, bt_package_t *new_tail)
{
	bt_package_t *tmp;

	tmp = old_tail->next;
	old_tail->next = new_tail;
	new_tail->next = tmp;
}

static void add_content_to_package(bt_package_t *tmp, char *json, unsigned char type)
{
	tmp->json = json;
	tmp->type = type;
}

//order: 	0: 	NULL
//			1:	yes
//			2:	no
//Temp:		0:	NULL
//			1:	temperature
//accident:	0:	NULL
//			1:	accident
unsigned char add_send_package(unsigned char order, unsigned char Temp, unsigned char accident)
{
	bt_package_t *package_tmp;
	char *js_buf;
	unsigned char type_buf = 0xFF;

	package_tmp = (bt_package_t *)malloc(sizeof(bt_package_t));
	if(package_tmp == NULL)
	{
		printf("package_tmp malloc error!\r\n");
		return 1;
	}
	memset(package_tmp, 0, sizeof(bt_package_t));

	printf("order: %d, Temp: %d, accident: %d\r\n", order, Temp, accident);
	js_buf =  js_compose(order, Temp, accident);
	type_buf = accident? 0:(order? 1:2);
	add_content_to_package(package_tmp, js_buf, type_buf);

	switch(type_buf)
	{
		case 0:
				add_to_tail(&ACCIDENT_HEAD, package_tmp);
				break;
		case 1:
				add_to_tail(&ORDER_HEAD, package_tmp);
				break;
		case 2:
				add_to_tail(&TEMP_HEAD, package_tmp);
				break;
		default:
				printf("package type error!\r\n");
	}
	PENDING_PACKAGE++;
	return 0;
}

void delete_send_package_head(void)
{
	bt_package_t *head = &ACCIDENT_HEAD, *tmp;

	while(head->next != NULL)
	{
		if(head->type != 0xFF)
		{
			tmp->next = head->next;
			free(head->json);
			free(head);
			PENDING_PACKAGE--;
			break;
		}
		tmp = head;
		head = head->next;
	}
}

unsigned int get_pending_package_num(void)
{
	return PENDING_PACKAGE;
}
