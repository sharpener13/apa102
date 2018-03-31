#include <unistd.h>
#include "debug.h"
#include "display.h"

#define SPI_DEVICE  "/dev/spidev0.0"
#define SPI_SPEED   10000000


static const display_module_config_t display_modules[] =
{
    {
        /* const char              **/ .name       = "1",
        /* display_module_anchor_t  */ .anchor     = DISPLAY_ANCHOR_TOPLEFT,
        /* display_position_t       */ .position   = {0, 0},
        /* display_size_t           */ .size       = {32, 8},
    },
    {
        /* const char              **/ .name       = "2",
        /* display_module_anchor_t  */ .anchor     = DISPLAY_ANCHOR_TOPLEFT,
        /* display_position_t       */ .position   = {0, 8},
        /* display_size_t           */ .size       = {32, 8},
    },
};


static const display_config_t display_config =
{
    /* const char             **/ .spi_device   = SPI_DEVICE,
    /* int                     */ .spi_speed    = SPI_SPEED,
    /* diplay_module_config_t **/ .modules      = display_modules,
    /* int                     */ .module_count = sizeof(display_modules) / sizeof(display_module_config_t),
};


static uint32_t get_color(int index)
{
    switch (index % 8)
    {
        case 0: return 0xffff0000;
        case 1: return 0xff00ff00;
        case 2: return 0xff0000ff;
        case 3: return 0xffff00ff;
        case 4: return 0xffffff00;
        case 5: return 0xff00ffff;
        case 6: return 0xff777777;
        case 7: return 0xffffffff;
    }

    return 0;
}

int sinus[] = 
{
    1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 27, 28, 29, 29, 30, 30, 30, 
    31, 31, 31, 31, 31, 31, 31, 
    30, 30, 30, 29, 29, 28, 27, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 4, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1,
};

const int sin_count = sizeof(sinus) / sizeof(int);

int main(int argc, char *argv[])
{
    display_t display;    
    int s, b;

    debug_init();
    display_init(&display, &display_config);


    s = 0;
    while (1)
    {
        b = sinus[s++];
        if (s >= sin_count) 
        {
            s = 0;
            usleep(1000 * 1000);
        }

        display_set_brightness(&display, b);
        display_begin_frame(&display, false);
        {
            display_position_t topleft = {0, 0};
            display_position_t btmright = {31, 15};
            int i = 0;

            do
            {
                int p;
                uint32_t c = get_color(i++);
                
                for (p = topleft.x; p <= btmright.x; ++p)
                {
                    display_set_pixel(&display, p, topleft.y, c, APA102_PIX_MODE_COPY);
                    display_set_pixel(&display, p, btmright.y, c, APA102_PIX_MODE_COPY);
                }

                for (p = topleft.y + 1; p < btmright.y; ++p)
                {
                    display_set_pixel(&display, topleft.x, p, c, APA102_PIX_MODE_COPY);
                    display_set_pixel(&display, btmright.x, p, c, APA102_PIX_MODE_COPY);
                }

                topleft.x += 2;
                topleft.y += 2;

                btmright.x -= 2;
                btmright.y -= 2;
            } while (   (btmright.x > topleft.x)
                     && (btmright.y > topleft.y));
        }
        display_finish_frame(&display);

        usleep(10 * 1000);
    }

    display_done(&display);
    debug_done();
}
