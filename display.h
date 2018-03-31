/*************************************************************************//**
 * @file display.c
 *
 *     APA102 flexible PCB display support.
 *
 ****************************************************************************/
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "apa102.h"


/*****************************************************************************
 * Public types
 ****************************************************************************/


typedef enum display_module_anchor_tt
{
    DISPLAY_ANCHOR_TOPLEFT,
    DISPLAY_ANCHOR_TOPRIGHT,
    DISPLAY_ANCHOR_BTMRIGHT,
    DISPLAY_ANCHOR_BTMLEFT,
} display_module_anchor_t;


typedef struct display_position_tt
{
    int x;
    int y;
} display_position_t;


typedef struct display_size_tt
{
    int width;
    int height;
} display_size_t;


typedef struct display_module_config_tt
{
    const char              *name;
    display_module_anchor_t  anchor;
    display_position_t       position;
    display_size_t           size;
} display_module_config_t;


struct display_module_tt;


typedef struct display_module_tt
{
    const display_module_config_t *config;
    int                            led_offset;
    struct display_module_tt      *next;
} display_module_t;


typedef struct display_config_tt
{
    const char                    *spi_device;   /**< SPI Device name */
    int                            spi_speed;    /**< SPI Speed in Hz */
    const display_module_config_t *modules;
    int                            module_count;
} display_config_t;


typedef struct display_tt
{
    const display_config_t *config;
    display_module_t       *modules;
    apa102_config_t         led_config;
    apa102_t                leds;
} display_t;


/*****************************************************************************
 * Public prototypes
 ****************************************************************************/
int  display_init          (display_t *display, const display_config_t *config);
int  display_done          (display_t *display);
int  display_begin_frame   (display_t *display, bool copy_last);
int  display_finish_frame  (display_t *display);
int  display_set_pixel     (display_t *display, int x, int y, uint32_t argb, apa102_pix_mode_t mode);
int  display_get_pixel     (display_t *display, int x, int y, uint32_t *argb);
void display_clear         (display_t *display);
void display_fill          (display_t *display, uint32_t argb);
void display_set_brightness(display_t *display, uint8_t brightness);


#endif
/*****************************************************************************
 * End of file
 ****************************************************************************/
