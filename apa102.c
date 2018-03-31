/*************************************************************************//**
 * @file apa102.c
 *
 *     APA102 LED chain rendering support library.
 *
 ****************************************************************************/
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include "colors.h"
#include "fifo.h"
#include "sync_fifo.h"
#include "debug.h"
#include "apa102spi.h"
#include "apa102.h"


/*****************************************************************************
 * Private macros
 ****************************************************************************/
#define DEFAULT_SPI_DEVICE  "/dev/spidev0.0"
#define DEFAULT_SPI_SPEED   20000000
#define DEFAULT_PIXEL_COUNT 146
#define DEFAULT_BRIGHTNESS  8

#define PIXEL_LEN        (32 / 8) /* 32 bits per pixel as ABGR, where A is 0b111aaaaa */
#define FRAME_START_LEN  (32 / 8) /* 32 bits for frame start */
#define FRAME_START_POS  0
#define FRAME_DATA_POS   FRAME_START_LEN
#define FRAME_COUNT      8

#define BRIGHT_MAX  31
#define BRIGHT_MASK 0x1f
#define BRIGHT_RAW  0xe0
#define BRIGHT_PICK(desired, def) (((desired) <= (BRIGHT_MAX)) ? (desired) : ((def) & BRIGHT_MASK))


/*****************************************************************************
 * Private prototypes
 ****************************************************************************/
static int       get_frame_data_len(apa102_t *self);
static int       get_frame_end_pos (apa102_t *self);
static int       get_frame_end_len (apa102_t *self);
static int       get_frame_len     (apa102_t *self);
static uint8_t **create_frames     (apa102_t *self);
static void      delete_frames     (apa102_t *self);
static void      write_frame_start (apa102_t *self, uint8_t *frame);
static void      write_frame_data  (apa102_t *self, uint8_t *frame);
static void      write_frame_end   (apa102_t *self, uint8_t *frame);
static int       get_pixel_pos     (apa102_t *self, int pixel);


/*****************************************************************************
 * Private functions
 ****************************************************************************/


static int get_frame_data_len(apa102_t *self)
{
    return self->config->pixel_count * PIXEL_LEN;
}


static int get_frame_end_pos(apa102_t *self)
{
    return FRAME_START_LEN + get_frame_data_len(self);
}


static int get_frame_end_len(apa102_t *self)
{
    /* 
     * align to four byte tuples -----------------------------------------------+
     * align to bytes --------------------------------------------+             |
     * compensate leading zeros ------------------------+         |             |
     * minimal bit count required -----------------+    |         |             |
     *                                             |    |         |             |
     *                                             V    V         V             V  */
    return ((((((self->config->pixel_count + 1) / 2) + 32 + 7) / 8) + 3) / 4) * 4;
}


static int get_frame_len(apa102_t *self)
{
    return FRAME_START_LEN + get_frame_data_len(self) + get_frame_end_len(self);
}


static void init_frame(apa102_t *self, uint8_t *frame)
{
    write_frame_start(self, frame);
    write_frame_data(self, frame);
    write_frame_end(self, frame);
}


static uint8_t **create_frames(apa102_t *self)
{
    int       frame_len = get_frame_len(self);
    uint8_t **frames    = (uint8_t **)malloc(FRAME_COUNT * sizeof(uint8_t *));
    int       i;

    self->frame_len = frame_len;

    for (i = 0; i < FRAME_COUNT; ++i)
    {
        uint8_t *frame = (uint8_t *)malloc(frame_len);

        init_frame(self, frame);
        frames[i] = frame;
    }

    return frames;
}


static void delete_frames(apa102_t *self)
{
    uint8_t **frames = self->frame_pool;
    int       i;

    for (i = 0; i < FRAME_COUNT; ++i)
    {
        free(frames[i]);
        frames[i] = NULL;
    }
    free(frames);
}


static void write_frame_start(apa102_t *self, uint8_t *frame)
{
    memset(frame + FRAME_START_POS, 0x00, FRAME_START_LEN);
}


static void write_frame_data(apa102_t *self, uint8_t *frame)
{
    int pos = FRAME_DATA_POS;
    int i;

    for (i = 0; i < self->config->pixel_count; ++i)
    {
        frame[pos + 0] = 0xe0;
        frame[pos + 1] = 0x00;
        frame[pos + 2] = 0x00;
        frame[pos + 3] = 0x00;
        pos += PIXEL_LEN;
    }
}


static void write_frame_end(apa102_t *self, uint8_t *frame)
{
    int pos = get_frame_end_pos(self);
    int len = get_frame_end_len(self);
    int cnt = len / 4;
    int i;
    uint8_t marker = 0;

#if 0
    memset(frame + pos, 0xe0, len);
#else
    for (i = 0; i < cnt; ++i)
    {
        frame[pos + 0] = 0xe0; marker++;
        frame[pos + 1] = marker++;
        frame[pos + 2] = marker++;
        frame[pos + 3] = marker++;
        pos += PIXEL_LEN;
    }
#endif
}


static int get_pixel_pos(apa102_t *self, int pixel)
{
    return ((pixel >= 0) && (pixel < self->config->pixel_count)) ? (FRAME_START_LEN + (pixel * PIXEL_LEN)) : -1;
}


static void *renderer(void *arg)
{
    apa102_t *self = (apa102_t *)arg;

    self->is_renderer_running = true;

    while (true)
    {
        void *item = NULL;

        if (   (sync_fifo_get(&self->full_frames, &item, self->is_renderer_running) == 0)
            && (item != NULL))
        {
            uint8_t *frame = (uint8_t *)item;

            DEBUG_DMP(stdout, frame, self->frame_len, 0, "Rendering frame", NULL);
            apa102spi_update(frame, self->frame_len);
            sync_fifo_put(&self->free_frames, item, true);
        }
        else
            break;
    }

    return NULL;
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/


/*************************************************************************//**
 * Initialize the context
 *
 * Some of the context members are supposed to be setup prior this call, the
 * private part is initialized by this.
 *
 * @param[in,out]    self    APA102 chain context
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int apa102_init(apa102_t *self, const apa102_config_t *config)
{
    int i;

    DEBUG_FMT(stderr, "Initializing: %s, %d Hz, %d pix, brightness %d\n", config->spi_device, config->spi_speed, config->pixel_count, config->brightness);

    self->config       = config;
    self->brightness   = config->brightness;
    self->active_frame = NULL;
    self->prev_frame   = NULL;
    self->frame_pool   = create_frames(self);

    DEBUG_MSG(stderr, "Preparing FIFOs...\n");
    sync_fifo_init(&self->free_frames, FRAME_COUNT, "free_frames");
    sync_fifo_init(&self->full_frames, FRAME_COUNT, "full_frames");

    for (i = 0; i < FRAME_COUNT; ++i)
    {
        fifo_put(&self->free_frames.raw, (void *)(self->frame_pool[i]));
    }

    DEBUG_MSG(stderr, "Creating renderer...\n");
    pthread_create(&self->th_renderer, NULL, renderer, (void *)self);

    DEBUG_MSG(stderr, "Opening SPI...\n");
    return apa102spi_open(self->config->spi_device, self->config->spi_speed);
}


/*************************************************************************//**
 * Finalize the context
 *
 * Also release any allocated resources.
 *
 * @param[in,out]    self    APA102 chain context
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int apa102_done(apa102_t *self)
{
    int ret = -1;

    DEBUG_MSG(stderr, "Finalizing...\n");

    self->is_renderer_running = false;

    sync_fifo_put(&self->full_frames, NULL, true);
    pthread_join(self->th_renderer, NULL);

    ret = apa102spi_close();

    sync_fifo_done(&self->full_frames);
    sync_fifo_done(&self->free_frames);

    delete_frames(self);
    self->frame_pool = NULL;

    return ret;
}


/*************************************************************************//**
 * Prepare for rendering of the new frame
 *
 * Any pixel manipulation is supposed to be done between begin/finish frame
 * calls.
 *
 * @param[in,out]    self         APA102 chain context
 * @param[in]        copy_last    Copy the previous frame content or start with
 *                                empty one
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int apa102_begin_frame(apa102_t *self, bool copy_last)
{
    void    *item       = NULL;
    uint8_t *prev_frame = self->prev_frame;
    uint8_t *curr_frame;

    DEBUG_MSG(stderr, "Starting new frame...\n");

    sync_fifo_get(&self->free_frames, &item, true);
    curr_frame = (uint8_t *)item;

    if (copy_last && (prev_frame != NULL))
        memcpy(item, (void *)prev_frame, self->frame_len);
    else
        init_frame(self, curr_frame);

    self->active_frame = curr_frame;

    return 0;
}


/*************************************************************************//**
 * Finish rendering of the frame
 *
 * Finished frame is requested to be renderred (means to be sent over SPI to
 * LEDs).
 *
 * @param[in,out]    self    APA102 chain context
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int apa102_finish_frame(apa102_t *self)
{
    uint8_t *curr_frame = self->active_frame;

    self->prev_frame   = curr_frame;
    self->active_frame = NULL;

    return sync_fifo_put(&self->full_frames, (void *)curr_frame, true);
}


/*************************************************************************//**
 * Change pixel color
 *
 * Use the AARRGGBB format for the pixel color, where for AA value above 0x1f is
 * considered to be invalid and the detfault one is used.
 *
 * @param[in,out]    self     APA102 chain context
 * @param[in]        pixel    LED offset in the chain
 * @param[in]        argb     Desired color
 * @param[in]        mode     Pixel combination mode (might need some additional
 *                            love to be more useful)
 *
 * @return    zero on success, nonzero otherwise (pixel out of range)
 *
 ****************************************************************************/
int apa102_set_pixel(apa102_t *self, int pixel, uint32_t argb, apa102_pix_mode_t mode)
{
    uint8_t *frame = self->active_frame;
    int      pos   = get_pixel_pos(self, pixel);

    if (frame == NULL)
    {
        DEBUG_MSG(stderr, "Frame not started!\n");
        return -1;
    }

    if (pos < 0)
    {
        DEBUG_FMT(stderr, "Ignoring request to pixel %d, value 0x%08x!\n", pixel, argb);
        return -2;
    }

    
    DEBUG_FMT(stdout, "Setting pixel %3d, value 0x%02x_%02x_%02x_%02x, pos %d\n", pixel, (argb >> 24) & 0xff, (argb >> 16) & 0xff, (argb >> 8) & 0xff, (argb >> 0) & 0xff, pos);
    switch (mode)
    {
        case APA102_PIX_MODE_COPY:
            frame[pos + 0] = BRIGHT_RAW | BRIGHT_PICK(COL_ALP(argb), self->brightness);
            frame[pos + 1] = COL_BLU(argb);
            frame[pos + 2] = COL_GRN(argb);
            frame[pos + 3] = COL_RED(argb);
            break;

        case APA102_PIX_MODE_ADD:
        {
            uint8_t bright_old = frame[pos + 0] & BRIGHT_MASK;
            uint8_t bright_new = BRIGHT_PICK(COL_ALP(argb), self->brightness);

            frame[pos + 0] = BRIGHT_RAW | COL_ADD(bright_old, bright_new, BRIGHT_MAX);
            frame[pos + 1] = COL_ADD(frame[pos + 1], COL_BLU(argb), 255);
            frame[pos + 2] = COL_ADD(frame[pos + 2], COL_GRN(argb), 255);
            frame[pos + 3] = COL_ADD(frame[pos + 3], COL_RED(argb), 255);
            break;
        }

        case APA102_PIX_MODE_SUB:
        {
            uint8_t bright_old = frame[pos + 0] & BRIGHT_MASK;
            uint8_t bright_new = BRIGHT_PICK(COL_ALP(argb), self->brightness);

            frame[pos + 0] = BRIGHT_RAW | COL_SUB(bright_old, bright_new, 1);
            frame[pos + 1] = COL_SUB(frame[pos + 1], COL_BLU(argb), 1);
            frame[pos + 2] = COL_SUB(frame[pos + 2], COL_GRN(argb), 1);
            frame[pos + 3] = COL_SUB(frame[pos + 3], COL_RED(argb), 1);
            break;
        }

        case APA102_PIX_MODE_SUB2:
            frame[pos + 0] = BRIGHT_RAW | BRIGHT_PICK(COL_ALP(argb), self->brightness);
            frame[pos + 1] = COL_SUB2(frame[pos + 1], COL_BLU(argb), 1, 32);
            frame[pos + 2] = COL_SUB2(frame[pos + 2], COL_GRN(argb), 1, 32);
            frame[pos + 3] = COL_SUB2(frame[pos + 3], COL_RED(argb), 1, 32);
            break;

        case APA102_PIX_MODE_INV2:
            frame[pos + 0] = BRIGHT_RAW | BRIGHT_PICK(COL_ALP(argb), self->brightness);
            frame[pos + 1] = COL_INV2(frame[pos + 1], COL_BLU(argb), 1);
            frame[pos + 2] = COL_INV2(frame[pos + 2], COL_GRN(argb), 1);
            frame[pos + 3] = COL_INV2(frame[pos + 3], COL_RED(argb), 1);
            break;

        case APA102_PIX_MODE_XOR:
            frame[pos + 0] = BRIGHT_RAW | BRIGHT_PICK(COL_ALP(argb), self->brightness);
            frame[pos + 1] ^= COL_BLU(argb);
            frame[pos + 2] ^= COL_GRN(argb);
            frame[pos + 3] ^= COL_RED(argb);
            break;
    }

    return 0;
}


/*************************************************************************//**
 * Get the current pixel color
 *
 * @param[in,out]    self     APA102 chain context
 * @param[in]        pixel    LED offset in the chain
 * @param[out]       argb     Read color
 *
 * @return    zero on success, nonzero otherwise (pixel out of range)
 *
 ****************************************************************************/
int apa102_get_pixel(apa102_t *self, int pixel, uint32_t *argb)
{
    uint8_t *frame = self->active_frame;
    int      pos   = get_pixel_pos(self, pixel);
    int      ret   = 0;

    if (pos >= 0)
        *argb = COL_ARGB(BRIGHT_MASK & frame[pos + 0], frame[pos + 3], frame[pos + 2], frame[pos + 1]);
    else
        ret = -1;

    return ret;
}


/*************************************************************************//**
 * Switch off all LEDs in the chain
 *
 * @param[in,out]    self    APA102 chain context
 *
 ****************************************************************************/
void apa102_clear(apa102_t *self)
{
    uint8_t *frame = self->active_frame;
    int      pos   = FRAME_DATA_POS;
    int      i;

    for (i = 0; i <  self->config->pixel_count; ++i)
    {
        frame[pos + 0] = 0xe0;
        frame[pos + 1] = 0x00;
        frame[pos + 2] = 0x00;
        frame[pos + 3] = 0x00;
        pos += 4;
    }
}


/*************************************************************************//**
 * Change color of all LEDs in the chain
 *
 * @param[in,out]    self    APA102 chain context
 * @param[in]        argb    Desired color
 *
 ****************************************************************************/
void apa102_fill(apa102_t *self, uint32_t argb)
{
    int i;

    for (i = 0; i < self->config->pixel_count; ++i)
    {
        apa102_set_pixel(self, i, argb, APA102_PIX_MODE_COPY);
    }
}


/*************************************************************************//**
 * Change brightness of all LEDs in the chain
 *
 * This updates current frame and also the default value for next frames.
 *
 * @param[in,out]    self          APA102 chain context
 * @param[in]        brightness    Desired brightness level 0-31
 *
 ****************************************************************************/
void apa102_set_brightness(apa102_t *self, uint8_t brightness)
{
    int pos = FRAME_DATA_POS;
    int i;

    if (brightness > BRIGHT_MAX)
        brightness = BRIGHT_MAX;

    self->brightness = brightness;

    if (self->active_frame != NULL)
    {
        uint8_t *frame = self->active_frame;
        uint8_t  raw   = BRIGHT_RAW | brightness;

        for (i = 0; i < self->config->pixel_count; ++i)
        {
            frame[pos + 0] = raw;
            pos += PIXEL_LEN;
        }
    }
}


/*****************************************************************************
 * End of file
 ****************************************************************************/
