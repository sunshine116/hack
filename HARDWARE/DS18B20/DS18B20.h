#ifndef __DS18B20_H_
#define __DS18B20_H_

#define DQ_in 	PBin(2)
#define DQ_out 	PBout(2)

void ReadTemperature(unsigned char *symbol, unsigned int *integer, unsigned char *dot);

#endif
