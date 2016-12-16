#ifndef __JS_PARSE_H_
#define __JS_PARSE_H_

void parse_js(char *js);
unsigned char bt_resp_send(unsigned char order, unsigned char Temp, unsigned char accident);
unsigned char bt_receive(void);
void dir_display_poll(void);

#endif
