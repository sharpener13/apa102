/*************************************************************************//**
 * @file display.c
 *
 *     APA102 flexible PCB display support.
 *
 *   The LED chain in the module is zig-zag organized. So the normal (x, y)
 * coordinates are translated to this situation. The translation also depends
 * on the chosen module orientation, the first (input) LED is considered as
 * anchor point.
 *
 * E.g. for display having two 5x3 LED modules,
 *                   "M1"                      "M2"
 *
 *   y\x       0   1   2   3   4         5   6   7   8   9
 *   0    in->00->01->02->03->04--+  +->10->11->12->13->14-->out
 *                                |  |
 *   1     +--09<-08<-07<-06<-05<-+  +--09<-08<-07<-06<-05<-+
 *         |                                                |
 *   2     +->10->11->12->13->14-->o/i->00->01->02->03->04--+
 *
 * the coordinates (x, y) = [3, 1] are translated to M1's LED 06,
 * the coordinates (x, y) = [8, 2] are translated to M2's LED 03.
 *
 *     So, there might be several modules mapped to the virtual display space.
 * Each module maps itself via position, size and orientation. These
 * properties are considered when pixel change request is handled, appropriate
 * module is found and given LED updated. The order the modules are defined is
 * significant and must correspond to the physical connection order.
 *
 * Current case would be:           M1         M2
 *     - position (x, y)          [0, 0]     [5, 0]
 *     - size     (width, height) [5, 3]     [5, 3]
 *     - anchor                   top-left   bottom-left
 *
 ****************************************************************************/

#include <malloc.h>
#include "debug.h"
#include "apa102.h"
#include "display.h"


/*****************************************************************************
 * Private functions
 ****************************************************************************/


static int get_total_pixel_count(const display_module_config_t *modules, int count)
{
    int i;
    int pixels = 0;

    for (i = 0; i < count; ++i)
    {
        const display_module_config_t *m = &modules[i];
        pixels += m->size.width * m->size.height;
    }

    return pixels;
}


static bool is_module_hit(const display_module_config_t *module, int x, int y)
{
    return x >= module->position.x 
        && x <  module->position.x + module->size.width
        && y >= module->position.y
        && y <  module->position.y + module->size.height;
}


static display_module_t *get_module_by_position(display_module_t *modules, int x, int y)
{
    display_module_t *m = modules;

    while (m != NULL)
    {
        if (is_module_hit(m->config, x, y))
            return m;
        m = m->next;
    }

    return NULL;
}


static int get_led_offset(display_module_t *module, int x, int y)
{
    int zx = x - module->config->position.x;
    int zy = y - module->config->position.y;
    int w  = module->config->size.width;
    int h  = module->config->size.height;

    int minor = 0;
    int major = 0;

    switch (module->config->anchor)
    {
        case DISPLAY_ANCHOR_TOPLEFT:
        {
            minor = zx % w;
            major = zy * w;            

            if (zy & 1)
                minor = w - 1 - minor;
            break;
        }

        case DISPLAY_ANCHOR_BTMRIGHT:
        {
            int ry = (h - 1) - zy;

            minor = zx % w;
            major = ry * w;

            if (!(ry & 1))
                minor = w - 1 - minor;
            break;
        }

        case DISPLAY_ANCHOR_BTMLEFT:
        {
            minor = zy % h;
            major = zx * h;

            if (!(zx & 1)) 
                minor = h - 1 - minor;
            break;
        }

        case DISPLAY_ANCHOR_TOPRIGHT:
        {
            int rx = (w - 1) - zx;

            minor = zy % h;
            major = rx * h;

            if (rx & 1)
                minor = h - 1 - minor;
            break;
        }
    }

    return module->led_offset + major + minor;
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/


int display_init(display_t *display, const display_config_t *config)
{
    int               i;
    int               offset = 0;
    int               mcnt   = config->module_count;
    display_module_t *prev   = NULL;

    DEBUG_MSG(stderr, "Initializing display...\n");
    display->config                 = config;
    display->led_config.spi_device  = display->config->spi_device;
    display->led_config.spi_speed   = display->config->spi_speed;
    display->led_config.pixel_count = get_total_pixel_count(display->config->modules, display->config->module_count);
    display->led_config.brightness  = 0;

    DEBUG_MSG(stderr, "Initializing modules...\n");
    display->modules = (display_module_t *)malloc(mcnt * sizeof(display_module_t));

    for (i = 0; i < mcnt; ++i)
    {
        const display_module_config_t *mcfg = &config->modules[i];
        display_module_t              *m    = &display->modules[i];

        m->config = mcfg;
        m->next = NULL;

        if (prev != NULL)
        {
            offset += prev->config->size.width * prev->config->size.height;
            prev->next = m;
        }

        m->led_offset = offset;
        prev = m;
    }

    DEBUG_MSG(stderr, "Initializing APA102...\n");
    return apa102_init(&display->leds, &display->led_config);
}


int display_done(display_t *display)
{
    apa102_done(&display->leds);
    free(display->modules);

    return 0;
}


int display_begin_frame(display_t *display, bool copy_last)
{
    return apa102_begin_frame(&display->leds, copy_last);
}


int display_finish_frame(display_t *display)
{
    return apa102_finish_frame(&display->leds);
}


int display_set_pixel(display_t *display, int x, int y, uint32_t argb, apa102_pix_mode_t mode)
{
    DEBUG_FMT(stderr, "Setting pixel [%d, %d]\n", x, y);

    display_module_t *m = get_module_by_position(display->modules, x, y);
    if (m != NULL)
    {
        int pixel = get_led_offset(m, x, y);
        return apa102_set_pixel(&display->leds, pixel, argb, mode);
    }
    else
    {
        DEBUG_MSG(stderr, "Module not found for that position!\n");
    }

    return -1;
}


int display_get_pixel(display_t *display, int x, int y, uint32_t *argb)
{
    return -1;
}


void display_clear(display_t *display)
{
    apa102_clear(&display->leds);
}


void display_fill(display_t *display, uint32_t argb)
{
    apa102_fill(&display->leds, argb);
}


void display_set_brightness(display_t *display, uint8_t brightness)
{
    apa102_set_brightness(&display->leds, brightness);
}


/*****************************************************************************
 * End of file
 ****************************************************************************/
