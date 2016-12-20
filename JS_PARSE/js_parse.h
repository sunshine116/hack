#ifndef __JS_PARSE_H_
#define __JS_PARSE_H_

#define JS_BUF_LEN   128

unsigned char parse_js(char *js);
char *js_compose(unsigned char order, unsigned char Temp, unsigned char accident);
void process_server_cmd(void);
void reset_orderId(void);

#endif
