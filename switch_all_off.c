#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "apa102.h"


#define SPI_DEVICE  "/dev/spidev0.0"
#define SPI_SPEED   10000000
#define PIXEL_COUNT 512


static apa102_config_t config =
{
    .spi_device  = SPI_DEVICE,
    .spi_speed   = SPI_SPEED,
    .pixel_count = PIXEL_COUNT,
    .brightness  = 0,
};
static apa102_t leds;


int main(int argc, char *argv[])
{
    if (argc > 1)
        config.pixel_count = atoi(argv[1]);            

    if (apa102_init(&leds, &config) == 0)
    {
        int i;

        apa102_begin_frame(&leds, false);
        for (i = 0; i < config.pixel_count; ++i)
        {
            uint32_t col = (i == 0) ? 0x563412 : ((i == config.pixel_count - 1) ? 0x214365 : 0);
            apa102_set_pixel(&leds, i, col, APA102_PIX_MODE_COPY);
        }
        apa102_finish_frame(&leds);
        apa102_done(&leds);
    }
    else
        fprintf(stderr, "Cannot init APA102 library!\n");

    return 0;
}
