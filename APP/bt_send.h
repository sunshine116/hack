#ifndef __BT_SEND_H_
#define __BT_SEND_H_

typedef struct bt_package{
	char *json;
	unsigned char type; //0: accident 1: order  2: dir  3: temp
	struct bt_package *next;
}bt_package_t;

void add_order_package(unsigned char order_response);
unsigned char add_send_package(unsigned char order, unsigned char Temp, unsigned char accident);
void delete_send_package_head(void);
unsigned int get_pending_package_num(void);
void bt_send(void);


#endif

