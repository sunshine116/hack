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

/**
 * 使用双向链表表示需要发送的package，其中accident作为head，其次是order，最后是temp。accident链表的尾部指向order的头部，order
 * 的尾部指向temp的头部。新来的package加入到每个对应类型链表的最后面，所以链表头是不变的。在发送的时候，从accident头（也就是整个
 * 链表的头）开始搜索，如果type不为0xFF（accident头，order头和temp头的类型固定为0xFF)，就发送整个数据包，并将其从链表中删除。
 */
static bt_package_t *TEMP_HEAD;
static bt_package_t *ORDER_HEAD;
static bt_package_t *ACCIDENT_HEAD;

void init_send_package(void)
{
	ACCIDENT_HEAD = (bt_package_t *)malloc(sizeof(bt_package_t));
	ORDER_HEAD = (bt_package_t *)malloc(sizeof(bt_package_t));
	TEMP_HEAD = (bt_package_t *)malloc(sizeof(bt_package_t));

	ACCIDENT_HEAD->json = NULL;
	ACCIDENT_HEAD->type = 0xFF;
	ACCIDENT_HEAD->prev = NULL;
	ACCIDENT_HEAD->next = ORDER_HEAD;

	ORDER_HEAD->json = NULL;
	ORDER_HEAD->type = 0xFF;
	ORDER_HEAD->prev = ACCIDENT_HEAD;
	ORDER_HEAD->next = TEMP_HEAD;

	TEMP_HEAD->json = NULL;
	TEMP_HEAD->type = 0xFF;
	TEMP_HEAD->prev = ORDER_HEAD;
	TEMP_HEAD->next = NULL;
}

void add_order_package(unsigned char order_response)
{
	add_send_package(order_response, 1, 0);
	stop_order_poll();
	reset_orderId();
}

void bt_send(void)
{
	bt_package_t *head = ACCIDENT_HEAD;

	while(head->type == 0xFF)
	{
		head = head->next;
		if(head == NULL)
		{
			return;
		}
	}
	u2_printf("%s", head->json);
	printf("send json: %s\r\n", head->json);
	printf("\r\nPending Package: %d\r\n", PENDING_PACKAGE);
	delete_send_package(head);
}

static void add_to_tail(bt_package_t *tail, bt_package_t *new_tail)
{
	bt_package_t *tmp = tail;

	while(tmp->next != NULL)
	{
		tmp = tmp->next;
	}
	tmp->next = new_tail;
	new_tail->prev = tmp;
	new_tail->next = NULL;
}

static void add_to_head(bt_package_t *head, bt_package_t *new_head)
{
	bt_package_t *prev_tmp, *head_tmp = head;

	head_tmp = head;
	prev_tmp = head_tmp->prev;
	new_head->prev = prev_tmp;
	prev_tmp->next = new_head;

	head_tmp->prev = new_head;
	new_head->next = head_tmp;
}

static void add_content_to_package(bt_package_t *package, char *json, unsigned char type)
{
	bt_package_t *tmp;

	tmp = package;
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
				add_to_head(ORDER_HEAD, package_tmp);
				break;
		case 1:
				add_to_head(TEMP_HEAD, package_tmp);
				break;
		case 2:
				add_to_tail(TEMP_HEAD, package_tmp);
				break;
		default:
				printf("package type error!\r\n");
	}
	PENDING_PACKAGE++;
	return 0;
}

void delete_send_package(bt_package_t *package)
{
	bt_package_t *prev, *next;
	if(package->next == NULL)
	{
		prev = package->prev;
		prev->next = NULL;
	}
	else
	{
		prev = package->prev;
		next = package->next;
		prev->next = next;
		next->prev = prev;
	}
	free(package->json);
	free(package);
	PENDING_PACKAGE--;
}

unsigned int get_pending_package_num(void)
{
	return PENDING_PACKAGE;
}
