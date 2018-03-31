/*************************************************************************//**
 * @file apa102_test.c
 *
 *     Just quick test of APA102 libraries and related stuff.
 *
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "apa102.h"
#include "larson.h"
#include "debug.h"


/*****************************************************************************
 * Private macros
 ****************************************************************************/
#define SPI_DEVICE  "/dev/spidev0.0"
#define SPI_SPEED   10000000
#define PIXEL_COUNT 256
//#define PIXEL_COUNT 310
//#define PIXEL_COUNT 124
//#define PIXEL_COUNT 8
#define BRIGHTNESS 2


/*****************************************************************************
 * Private variables
 ****************************************************************************/


static const apa102_config_t config =
{
    .spi_device  = SPI_DEVICE,
    .spi_speed   = SPI_SPEED,
    .pixel_count = PIXEL_COUNT,
    .brightness  = BRIGHTNESS,
};

static volatile bool is_running = true;
static apa102_t leds;


/*****************************************************************************
 * Private functions
 ****************************************************************************/


static void sig_handler(int signo)
{
    if (signo == SIGINT)
    {
        DEBUG_MSG(stdout, "Aborting test...\n");
        is_running = false;
    }
}


static uint64_t get_us(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000000 + tv.tv_usec;
}


static void test1(void)
{
    int ofs = 0;

    DEBUG_MSG(stdout, "Starting test 1...\n");
    is_running = true;
    while (is_running)
    {
        int pix = 0;
        int col = ofs;

        while (pix < PIXEL_COUNT)
        {
            uint8_t r = 0;
            uint8_t g = 0;
            uint8_t b = 0;

            switch (col)
            {
                case 0: /* R */ r = 0xff; break;
                case 1: /* M */ b = 0xff; r = 0xff; break;
                case 2: /* G */ g = 0xff; break;
                case 3: /* C */ g = 0xff; b = 0xff; break;
                case 4: /* B */ b = 0xff; break;
                case 5: /* Y */ r = 0xff; g = 0xff; break;
                case 6: /* W */ r = 0xff; g = 0xff; b = 0xff; break;
            }
            col = (col + 1) % 7;
            DEBUG_FMT(stdout, "Setting pixel %d to A:0x%02x R:0x%02x G:0x%02x B:0x%02x\n", pix, BRIGHTNESS, r, g, b);

            apa102_begin_frame(&leds, false);
            apa102_set_pixel(&leds, pix, COL_ARGB(BRIGHTNESS, r, g, b), APA102_PIX_MODE_COPY);
            apa102_finish_frame(&leds);
            ++pix;
        }
        usleep(1000 * 1000);
        ofs = (ofs + 1) % 7;
    }
    DEBUG_MSG(stdout, "Finishing test 1.\n");
}


static uint32_t get_color(int pos)
{
    int     col = pos % 7;
    uint8_t r   = 0;
    uint8_t g   = 0;
    uint8_t b   = 0;

    switch (col)
    {
        case 0: /* R */ r = 0xff; break;
        case 1: /* M */ b = 0xff; r = 0xff; break;
        case 2: /* G */ g = 0xff; break;
        case 3: /* C */ g = 0xff; b = 0xff; break;
        case 4: /* B */ b = 0xff; break;
        case 5: /* Y */ r = 0xff; g = 0xff; break;
        case 6: /* W */ r = 0xff; g = 0xff; b = 0xff; break;
    }

    return COL_ARGB(BRIGHTNESS, r, g, b);
}


static void test11(void)
{
    int pos  = 0;
    int spot = PIXEL_COUNT;

    is_running = true;
    while (is_running)
    {
        uint32_t spot_col = get_color(spot - 1);
        int pix = 0;

        apa102_begin_frame(&leds, false);
        while (pix < PIXEL_COUNT)
        {
            uint32_t c = 0;

            if (pix == pos)
                c = spot_col;
            else if ((pix >= spot) /* && (pix < spot + 16) */) 
                c = get_color(pix);

            if (c)
                apa102_set_pixel(&leds, pix, c, APA102_PIX_MODE_COPY);

            ++pix;
        }
        apa102_finish_frame(&leds);

        usleep(5 * 1000);

        ++pos;
        if ((pos >= spot) || (pos >= PIXEL_COUNT))
        {
            pos = 0;
            if (--spot <= 0)
                spot = PIXEL_COUNT;
        }
    }
}


static void test2(void)
{
    is_running = true;
    if (is_running)
    {
        larson_t larsons[] =
        {
            {
                .pixels            = PIXEL_COUNT,
                .length            = PIXEL_COUNT,
                .position          = 0,
                .is_forward        = true,
                .is_bidirect       = false,
                .speed             = 1,
                .color             = 0xffff0000,
                .frame_update_time = 33 * 1000,
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
                .frame_update_time = 15 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
            {
                .pixels            = PIXEL_COUNT,
                .length            = 10,
                .position          = -PIXEL_COUNT / 2,
                .is_forward        = rand() & 1,
                .is_bidirect       = true,
                .speed             = 1,
                .color             = 0xff770000,
                .frame_update_time = 19 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
            {
                .pixels            = PIXEL_COUNT,
                .length            = 4,
                .position          = -PIXEL_COUNT / 4,
                .is_forward        = true,
                .is_bidirect       = false,
                .speed             = 1,
                .color             = 0xffffff00,
                .frame_update_time = 25 * 1000,
                .mode              = APA102_PIX_MODE_XOR,
            },
        };
        int      frame = 0;
        int      count = sizeof(larsons) / sizeof(larson_t);
        uint64_t start;
        int      i;

        DEBUG_MSG(stdout, "Starting test 2...\n");

        apa102_set_brightness(&leds, BRIGHTNESS);

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
        DEBUG_MSG(stdout, "Finishing test 2.\n");
    }
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/


/*************************************************************************//**
 * Main.
 *
 * Entry point.
 *
 ****************************************************************************/
int main(int argc, char *argv[])
{
    debug_init();
    srand((unsigned int)get_us());
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        DEBUG_MSG(stderr, "Cannot catch SIGINT!\n");

    if (apa102_init(&leds, &config) == 0)
    {
        test1();
        test11();
        test2();
        apa102_done(&leds);
    }
    else
        DEBUG_MSG(stderr, "Cannot init APA102 library!\n");

    debug_done();

    return 0;
}


/*****************************************************************************
 * End of file
 ****************************************************************************/
