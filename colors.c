/*************************************************************************//**
 * @file colors.c
 *
 *     Various color related support stuff
 *
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "colors.h"


/*****************************************************************************
 * Public functions
 ****************************************************************************/


void col_change_init(col_change_t *ch)
{
    int i;

    if (ch->steps < 2)
        ch->steps = 2;

    ch->step  = 0;

    for (i = 0; i < 4; ++i)
    {
        int sh = i << 3; /* B, G, R, A */
        int c1 = 0xff & (ch->start >> sh);
        int c2 = 0xff & (ch->stop  >> sh);

        ch->bgra_diff[i] = c2 - c1;
    }
}


bool col_change_update(col_change_t *ch)
{
    if (ch->step < ch->steps)
    {
        uint32_t col = 0;
        int i;

        for (i = 0; i < 4; ++i)
        {
            int sh = i << 3; /* B, G, R, A */
            int c1 = 0xff & (ch->start >> sh);
            int in = (ch->bgra_diff[i] * ch->step) / (ch->steps - 1);
            int c2 = c1 + in;

            col |= (0xff & c2) << sh;
        }

        ch->step += 1;
        ch->current = col;
    }
    else
        ch->current = ch->start;

    return ch->step >= ch->steps;
}


uint32_t col_pick_random(void)
{
#if 0
    static const uint8_t components[] = {255, 128, 64, 32, 0};
    int      len = sizeof(components) / sizeof(uint8_t);
    uint32_t col = 0xff000000;
    int      zer = 0;
    int      i;

    for (i = 0; i < 3; ++i)
    {
        int sh = i << 3; /* B, G, R, A */
        int c2;

        c2 = components[rand() % len];
        if (c2 == 0)
        {
            zer += 1;
            if (zer == 2)
                --len;
        }

        col |= (0xff & c2) << sh;
    }

    return col;
#else
    static const uint32_t colors[] =
    {
        0xffff0000,
        0xff00ff00,
        0xff0000ff,
        0xffffff00,
        0xff00ffff,
        0xffff00ff,
    };
    int len = sizeof(colors) / sizeof(uint32_t);

    return colors[rand() % len];
#endif
}


/*****************************************************************************
 * End of file
 ****************************************************************************/
