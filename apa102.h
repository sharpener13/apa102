/*************************************************************************//**
 * @file apa102.h
 *
 *     APA102 LED chain rendering support library.
 *
 ****************************************************************************/
#ifndef __APA102_H__
#define __APA102_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "sync_fifo.h"


/*****************************************************************************
 * Public types
 ****************************************************************************/


/**
 * Pixel combination mode
 */
typedef enum apa102_pix_mode_tt
{
    APA102_PIX_MODE_COPY,
    APA102_PIX_MODE_ADD,
    APA102_PIX_MODE_SUB,
    APA102_PIX_MODE_SUB2,
    APA102_PIX_MODE_XOR,
    APA102_PIX_MODE_INV2,
} apa102_pix_mode_t;


/**
 *  APA102 context
 */
typedef struct apa102_tt
{
    /* Public */
    char         *spi_device;   /**< SPI Device name */
    int           spi_speed;    /**< SPI Speed in Hz */
    int           pixel_count;  /**< Number of leds in the chain */
    int           brightness;   /**< Default brightness (0:off - 31:max) */

    /* Private */
    uint8_t     **frame_pool;
    int           frame_len;
    uint8_t      *active_frame;
    sync_fifo_t   free_frames;
    sync_fifo_t   full_frames;
    pthread_t     th_renderer;
    bool          is_renderer_running;
    bool          is_init;
} apa102_t;


/*****************************************************************************
 * Public prototypes
 ****************************************************************************/
int  apa102_init          (apa102_t *self);
int  apa102_done          (apa102_t *self);
int  apa102_begin_frame   (apa102_t *self, bool copy_last);
int  apa102_finish_frame  (apa102_t *self);
int  apa102_set_pixel     (apa102_t *self, int pixel, uint32_t argb, apa102_pix_mode_t mode);
int  apa102_get_pixel     (apa102_t *self, int pixel, uint32_t *argb);
void apa102_clear         (apa102_t *self);
void apa102_fill          (apa102_t *self, uint32_t argb);
void apa102_set_brightness(apa102_t *self, uint8_t brightness);


#endif
/*****************************************************************************
 * End of file
 ****************************************************************************/
