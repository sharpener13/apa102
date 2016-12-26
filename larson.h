#ifndef __LARSON_H__
#define __LARSON_H__


#include <stdint.h>
#include "colors.h"


typedef struct larson_tt
{
    /* Public */
    int               pixels;
    int               length;
    int               position;
    bool              is_forward;
    bool              is_bidirect;
    int               speed;
    uint32_t          color;
    unsigned int      frame_update_time;
    apa102_pix_mode_t mode;

    /* Private */
    int              *bright;
    col_change_t      head_change;
    col_change_t      tail_change;
    uint64_t          last_update_time;
} larson_t;


void larson_init(larson_t *larson, uint64_t time);
void larson_done(larson_t *larson);
void larson_update(larson_t *larson, uint64_t time);
void larson_render(larson_t *larson, apa102_t *apa102);


#endif
