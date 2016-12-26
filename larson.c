/*************************************************************************//**
 * @file larson.c
 *
 *     Fancy LED pattern generation
 *
 ****************************************************************************/
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "apa102.h"
#include "larson.h"


/*****************************************************************************
 * Private functions
 ****************************************************************************/


static int *create_bright(int length)
{
    return (int *)malloc(length * sizeof(int));
}


static void destroy_bright(int *bright)
{
    free(bright);
}


static void init_bright(int *bright, int length)
{
    int i;

    for (i = 0; i < length; ++i)
    {
        double raw = pow(10, 3 - 2.0 * (i / (double)length));
        bright[i] = (int)round(raw);
    }
}


#if 0
static void dump_bright(int *bright, int length)
{
    int i;

    printf("larson bright: ");
    for (i = 0; i < length; ++i)
    {
        printf("%d%s", bright[i], i < (length - 1) ? "," : "");
    }
    printf("\n");
}
#endif


static void init_col_changes(larson_t *larson)
{
    int steps = larson->pixels / 4;

    larson->head_change.start = larson->head_change.stop;
    larson->head_change.stop  = col_pick_random();
    larson->head_change.steps = steps;

    larson->tail_change.start = larson->tail_change.current;
    larson->tail_change.stop  = col_pick_random();
    larson->tail_change.steps = steps;

    col_change_init(&larson->head_change);
    col_change_init(&larson->tail_change);
}


static uint8_t scale(int val, int old_max, int new_max)
{
    return (val * new_max) / old_max;
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/


/*************************************************************************//**
 * Initialize Larson
 *
 * As usual, public fields are expected to be set up prior this call.
 *
 * @param[in,out]    larson    Larson's context
 * @param[in]        time      Time of initialization, looks unused
 *
 ****************************************************************************/
void larson_init(larson_t *larson, uint64_t time)
{
    srand((unsigned int)time);

    larson->head_change.stop = larson->color;
    larson->tail_change.stop = col_pick_random();
    larson->bright           = create_bright(larson->length);

    init_bright(larson->bright, larson->length);
    init_col_changes(larson);
}


/*************************************************************************//**
 * Finalize Larson
 *
 * @param[in,out]    larson    Larson's context
 *
 ****************************************************************************/
void larson_done(larson_t *larson)
{
    destroy_bright(larson->bright);
}


/*************************************************************************//**
 * Update Larson's internal state
 *
 * If time is already up, the state is updated, otherwise the update is skipped.
 *
 * @param[in,out]    larson    Larson's context
 * @param[in]        time      Current time (in microseconds)
 *
 ****************************************************************************/
void larson_update(larson_t *larson, uint64_t time)
{
    unsigned int delay = time - larson->last_update_time;

    if (   (larson->last_update_time == 0)
        || (delay >= larson->frame_update_time))
    {
        int dir = larson->is_forward ? 1 : -1;

        col_change_update(&larson->tail_change);
        if (col_change_update(&larson->head_change))
            init_col_changes(larson);

        larson->position += dir * larson->speed;

        if (larson->is_forward)
        {
            if (larson->position - larson->length > larson->pixels)
            {
                if (larson->is_bidirect)
                {
                    larson->is_forward = false;
                    larson->position = larson->pixels - 1;
                }
                else
                {
                    larson->position = 0;
                }
            }
        }
        else
        {
            if (larson->position + larson->length < 0)
            {
                if (larson->is_bidirect)
                {
                    larson->is_forward = true;
                    larson->position = 0;
                }
                else
                {
                    larson->position = larson->pixels - 1;
                }
            }
        }

        larson->last_update_time = time;
    }
}


/*************************************************************************//**
 * Render Larson to the LED chain
 *
 * Based on Larson's internals, render him to the LEDs
 *
 * @param[in,out]    larson    Larson's context
 * @param[in]        apa102    APA102 context for rendering
 *
 ****************************************************************************/
void larson_render(larson_t *larson, apa102_t *apa102)
{
    int      dir     = larson->is_forward ? 1 : -1;
    int      pix     = larson->position;
    uint32_t col1    = larson->head_change.current;
    uint32_t col2    = larson->tail_change.current;
    int      cnt     = 0;
    uint8_t  last[3] = {0, 0, 0};

    col_change_t body_change = 
    {
        .start = col1,
        .stop  = col2,
        .steps = larson->length,
    };
    col_change_init(&body_change);

    while (cnt < larson->length)
    {
        int      bri = larson->bright[cnt];
        uint32_t body_col;
        uint8_t  r;
        uint8_t  g;
        uint8_t  b;

        col_change_update(&body_change);
        body_col = body_change.current;

        /* FIXME */
        r = scale(COL_RED(body_col) * bri, 255 * 1000, 255);
        g = scale(COL_GRN(body_col) * bri, 255 * 1000, 255);
        b = scale(COL_BLU(body_col) * bri, 255 * 1000, 255);

        if ((r == 0) && (g == 0) && (b == 0)) {r = last[0]; g = last[1]; b = last[2];}
        else {last[0] = r; last[1] = g; last[2] = b;}

        apa102_set_pixel(apa102, pix, COL_ARGB(0xff, r, g, b), larson->mode);

        pix -= dir;
        cnt += 1;
    }
}


/*****************************************************************************
 * End of file
 ****************************************************************************/
