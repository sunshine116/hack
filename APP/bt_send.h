#ifndef __BT_SEND_H_
#define __BT_SEND_H_

typedef struct bt_package{
	char *json;
	unsigned char type; //0: accident 1: order  2: dir  3: temp
	struct bt_package *next;
}bt_package_t;

void bt_send_order(unsigned char order_response);
unsigned char add_package(char *json, unsigned char type);
void delete_package(void);
bt_package_t *get_package_head(void);
unsigned char bt_send(void);


#endif

