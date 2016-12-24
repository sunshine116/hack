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

static unsigned int PENDING_PACKAGE = 0;

static bt_package_t *PACKAGE_HEAD;
static bt_package_t *PACKAGE_TAIL;
/**
 * 使用双向链表表示需要发送的package，其中ACCIDENT_HEAD作为PACKAGE_HEAD，其次是ORDER_HEAD，然后是TEMP_HEAD，最后是TEMP_TAIL。
 * ACCIDENT_HEAD链表的尾部指向ORDER_HEAD的头部，ORDER_HEAD的尾部指向TEMP_HEAD的头部，TEMP_HEAD的尾部指向TEMP_TAIL，新来的
 * package加入到每个对应类型链表的最后，所以链表头是不变的。在发送的时候，从accident头（也就是整个链表的头）开始搜索，如果type不
 * 为0xFF（ACCIDENT_HEAD，ORDER_HEAD，TEMP_HEAD和TEMP_TAIL的类型固定为0xFF)，就发送这个数据包，并将其从链表中删除。如果
 * PENDING_PACKAGE大于等于MAX_PENDING_PACKAGE_NUM，先从链表中删除最小优先级的数据包，然后把这个数据包加入到链表中。
 */
static bt_package_t *ACCIDENT_HEAD;
static bt_package_t *ORDER_HEAD;
static bt_package_t *TEMP_HEAD;
static bt_package_t *TEMP_TAIL;

void init_package_list(void)
{
	ACCIDENT_HEAD = (bt_package_t *)malloc(sizeof(bt_package_t));
	ORDER_HEAD = (bt_package_t *)malloc(sizeof(bt_package_t));
	TEMP_HEAD = (bt_package_t *)malloc(sizeof(bt_package_t));
	TEMP_TAIL= (bt_package_t *)malloc(sizeof(bt_package_t));

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
	TEMP_HEAD->next = TEMP_TAIL;

	TEMP_TAIL->json = NULL;
	TEMP_TAIL->type = 0xFF;
	TEMP_TAIL->prev = TEMP_HEAD;
	TEMP_TAIL->next = NULL;

	PACKAGE_HEAD = ACCIDENT_HEAD;
	PACKAGE_TAIL= TEMP_TAIL;
}

void add_order_package(unsigned char order_response)
{
	add_send_package(order_response, 1, 0);
	stop_order_poll();
	reset_orderId();
}

void send_max_priority_package(void)
{
	bt_package_t *tmp = PACKAGE_HEAD;

	while(tmp->type == 0xFF)
	{
		tmp = tmp->next;
		if(tmp == NULL)
		{
			printf("Have no pending package!\r\n");
			return;
		}
	}
	u2_printf("%s", tmp->json);
	printf("send json: %s\r\n", tmp->json);
	printf("\r\nPending Package: %d\r\n", PENDING_PACKAGE);
	delete_send_package(tmp);
}

static void delete_min_priority_package(void)
{
	bt_package_t *tmp = PACKAGE_TAIL;

	while(tmp->type != 0xFF)
	{
		tmp = tmp->prev;
		if(tmp == NULL)
		{
			printf("Can not fing package, no package!\r\n");
			return;
		}
	}
	delete_send_package(tmp);
}

static void add_before_head(bt_package_t *head, bt_package_t *new_head)
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

	if(PENDING_PACKAGE >= MAX_PENDING_PACKAGE_NUM)
	{
		if(order == 0 && accident == 0 && Temp != 0)
		{
			return 1;
		}
		delete_min_priority_package();
	}
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
				add_before_head(ORDER_HEAD, package_tmp);
				break;
		case 1:
				add_before_head(TEMP_HEAD, package_tmp);
				break;
		case 2:
				add_before_head(TEMP_TAIL, package_tmp);
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
	if(package->next != NULL && package->prev != NULL)
	{
		prev = package->prev;
		next = package->next;
		prev->next = next;
		next->prev = prev;
	}
	else if(package->next == NULL)
	{
		prev = package->prev;
		prev->next = NULL;
	}
	else if(package->prev == NULL)
	{
		next = package->next;
		next->prev = NULL;
	}
	free(package->json);
	free(package);
	PENDING_PACKAGE--;
}

unsigned int get_pending_package_num(void)
{
	return PENDING_PACKAGE;
}
