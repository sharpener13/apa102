/*************************************************************************//**
 * @file colors.c
 *
 *     Various color related support stuff
 *
 ****************************************************************************/
#ifndef __COLORS_H__
#define __COLORS_H__

#include <stdint.h>
#include <stdbool.h>


/*****************************************************************************
 * Public macros
 ****************************************************************************/
#define COL_ALP(x) (0xff & ((x) >> 24))
#define COL_RED(x) (0xff & ((x) >> 16))
#define COL_GRN(x) (0xff & ((x) >>  8))
#define COL_BLU(x) (0xff & ((x) >>  0))

#define COL_ARGB(a, r, g, b) ((uint32_t)((((uint32_t)(a)) << 24) \
                                       | (((uint32_t)(r)) << 16) \
                                       | (((uint32_t)(g)) <<  8) \
                                       | (((uint32_t)(b)) <<  0)))

#define COL_ADD(x, y, max) ((((x) + (y)) < (max)) ? ((x) + (y)) : (max))
#define COL_SUB(x, y, min) ((((x) - (y)) > (min)) ? ((x) - (y)) : (min))
#define COL_SUB2(x, y, min, trig) (((x) > (trig)) ? ((((x) - (y)) > (min)) ? ((x) - (y)) : (min)) : (y))
#define COL_INV2(x, y, trig) (((x) > (trig)) ? (255 - (x)) : (y)) 


/*****************************************************************************
 * Public types
 ****************************************************************************/


/**
 * Color change context
 */
typedef struct col_change_tt
{
    /* Public */
    uint32_t start;     /**< Starting color  */
    uint32_t stop;      /**< Ending color    */
    int      steps;     /**< Number of steps */
    uint32_t current;   /**< Current color   */

    /* Private */
    int      step;
    int      bgra_diff[4];
} col_change_t;


/*****************************************************************************
 * Public prototypes
 ****************************************************************************/
void     col_change_init(col_change_t *ch);
bool     col_change_update(col_change_t *ch);
uint32_t col_pick_random(void);


#endif
/*****************************************************************************
 * End of file
 ****************************************************************************/
