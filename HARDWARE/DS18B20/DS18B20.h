#ifndef __DS18B20_H_
#define __DS18B20_H_

#define DQ_in 	PBin(2)
#define DQ_out 	PBout(2)

void Temp_get(unsigned char *symbol, unsigned int *integer, unsigned char *dot);
void Temp_string_get(char *buf);
void temp_upload_poll(void);
void ReadTemperature(void);

#endif
