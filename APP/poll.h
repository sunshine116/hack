#ifndef __POLL_H_
#define __POLL_H_

void HC05_connect_poll(void);
void accident_sta_poll(void);
void order_poll(void);
void temp_upload_poll(void);
void bt_receive_poll(void);
void display_poll(void);
void bt_send_poll(void);
void order_poll_start(void);
void stop_order_poll(void);
unsigned char is_order_poll(void);

#endif



