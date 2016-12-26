#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "apa102.h"
#include "larson.h"


#define SPI_DEVICE  "/dev/spidev0.0"
#define SPI_SPEED   20000000
#define PIXEL_COUNT 360
#define BRIGHTNESS  8


static apa102_t leds =
{
    .spi_device  = SPI_DEVICE,
    .spi_speed   = SPI_SPEED,
    .pixel_count = PIXEL_COUNT,
    .brightness  = BRIGHTNESS,
};

static bool is_running = true;


static void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        printf("Exiting...\n");
        is_running = false;
    }
}


#if 0
static void delay_us(int us)
{
    const struct timespec time =
    {
        .tv_sec = 0,
        .tv_nsec = us * 1000,
    };

    nanosleep(&time, NULL);
}
#endif


static uint64_t get_us(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000 + tv.tv_usec;
}


static void test1(void)
{
    if (is_running)
    {
        int pix = 0;
        int col = 0;

        printf("Starting test 1...\n");
        while (pix < PIXEL_COUNT)
        {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            switch (col)
            {
                case 0: r = 0xff; break;
                case 1: g = 0xff; break;
                case 2: b = 0xff; break;
                case 3: r = 0xff; g = 0xff; break;
                case 4: g = 0xff; b = 0xff; break;
                case 5: b = 0xff; r = 0xff; break;
                case 6: r = 0xff; g = 0xff; b = 0xff; break;
            }
            col = (col + 1) % 7;

            apa102_begin_frame(&leds, true);
            apa102_set_pixel(&leds, pix, COL_ARGB(BRIGHTNESS, r, g, b), APA102_PIX_MODE_COPY);
            apa102_finish_frame(&leds);
            ++pix;
        }
        printf("Finishing test 1.\n");
    }
}


static void test2(void)
{
    if (is_running)
    {
        larson_t larsons[] =
        {
            {
                .pixels            = PIXEL_COUNT,
                .length            = 200,
                .position          = 0,
                .is_forward        = true,
                .is_bidirect       = false,
                .speed             = 1,
                .color             = 0xffff0000,
                .frame_update_time = 25 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
            {
                .pixels            = PIXEL_COUNT,
                .length            = 8,
                .position          = PIXEL_COUNT - 1,
                .is_forward        = false,
                .is_bidirect       = false,
                .speed             = 1,
                .color             = 0xff000077,
                .frame_update_time = 5 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
            {
                .pixels            = PIXEL_COUNT,
                .length            = 10,
                .position          = PIXEL_COUNT / 2,
                .is_forward        = rand() & 1,
                .is_bidirect       = true,
                .speed             = 1,
                .color             = 0xff770000,
                .frame_update_time = 4 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
            {
                .pixels            = PIXEL_COUNT,
                .length            = 4,
                .position          = PIXEL_COUNT / 4,
                .is_forward        = true,
                .is_bidirect       = false,
                .speed             = 2,
                .color             = 0xffffff00,
                .frame_update_time = 6 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
#if 0            
            {
                .pixels            = PIXEL_COUNT,
                .length            = 4,
                .position          = PIXEL_COUNT - PIXEL_COUNT / 4,
                .is_forward        = rand() & 1,
                .speed             = 2,
                .color             = 0xffff0000,
                .frame_update_time = 10 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
#endif            
        };
        int      frame = 0;
        int      count = sizeof(larsons) / sizeof(larson_t);
        uint64_t start;
        int      i;

        printf("Starting test 2...\n");

        apa102_set_brightness(&leds, 31);

        for (i = 0; i < count; ++i)
        {
            larson_init(larsons + i, get_us());
        }

        start = get_us();
        while (is_running)
        {
            apa102_begin_frame(&leds, false);
            {
                uint64_t time = get_us() - start;

                for (i = 0; i < count; ++i)
                {
                    larson_update(larsons + i, time);
                    larson_render(larsons + i, &leds);
                }
            }
            apa102_finish_frame(&leds);

            ++frame;
        }

        for (i = 0; i < count; ++i)
        {
            larson_done(larsons + i);
        }
        printf("Finishing test 2.\n");
    }
}


int main(int argc, char *argv[])
{
    srand((unsigned int)get_us());
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        fprintf(stderr, "\nCan't catch SIGINT!\n");

    apa102_init(&leds);
    test1();
    test2();
    apa102_done(&leds);

    return 0;
}
