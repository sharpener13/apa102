#ifndef __APA102SPI_H__
#define __APA102SPI_H__

#include <stdint.h>

int apa102spi_open(char *device, int speed_hz);
int apa102spi_close(void);
int apa102spi_update(uint8_t *data, int length);


#endif
