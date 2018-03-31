/*************************************************************************//**
 * @file larson.h
 *
 *     Fancy LED pattern generation
 *
 *  I have no clue why Larson. I just inherited it from similar python module
 *  trying to manipulate APA102 devices ;-)
 *
 ****************************************************************************/
#ifndef __LARSON_H__
#define __LARSON_H__

#include <stdint.h>
#include "colors.h"


/*****************************************************************************
 * Public types
 ****************************************************************************/


/**
 * Larson's context
 */
typedef struct larson_tt
{
    /* Public */
    int               pixels;            /**< Number of pixels in the chain */
    int               length;            /**< Larson's length in pixels     */
    int               position;          /**< Larson's starting position    */
    bool              is_forward;        /**< Moving forward (initially)    */
    bool              is_bidirect;       /**< Running back and forth        */
    bool              is_looping;        /**< Running in loop               */
    int               speed;             /**< 1: Not skipping leds, >1 skips*/
    uint32_t          color;             /**< Legacy, say "color seed" ;-)  */
    unsigned int      frame_update_time; /**< Expected delay between updates*/
    apa102_pix_mode_t mode;              /**< Pixel combination mode        */

    /* Private */
    int              *bright;
    col_change_t      head_change;
    col_change_t      tail_change;
    uint64_t          last_update_time;
} larson_t;


/*****************************************************************************
 * Public prototypes
 ****************************************************************************/
void larson_init(larson_t *larson, uint64_t time);
void larson_done(larson_t *larson);
void larson_update(larson_t *larson, uint64_t time);
void larson_render(larson_t *larson, apa102_t *apa102);


#endif
/*****************************************************************************
 * End of file
 ****************************************************************************/
