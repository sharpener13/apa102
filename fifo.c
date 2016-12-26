/*************************************************************************//**
 * @file fifo.c
 *
 *     Simple FIFO handling
 *
 ****************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include "fifo.h"


/*******************************************************************************
 * Public functions
 ******************************************************************************/


/*************************************************************************//**
 * Initialize FIFO
 *
 * @param[in,out]    fifo    FIFO context
 * @param[in]        size    Number of items the FIFO can hold
 *
 ****************************************************************************/
void fifo_init(fifo_t *fifo, int size)
{
    fifo->size   = size;
    fifo->data   = (void **)malloc((size + 1) * sizeof(void *));
    fifo->rd     = fifo->wr = 0;
}


/*************************************************************************//**
 * Finalize FIFO
 *
 * @param[in,out]    fifo    FIFO context
 *
 ****************************************************************************/
void fifo_done(fifo_t *fifo)
{
    free(fifo->data);
    fifo->data = NULL;
    fifo->size = 0;
}


/*************************************************************************//**
 * Insert item into FIFO
 *
 * @param[in,out]    fifo    FIFO context
 * @param[in]        item    Item to be inserted
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int fifo_put(fifo_t *fifo, void *item)
{
    int ret = -1;

    if (!FIFO_FULL(*fifo))
    {
        fifo->data[fifo->wr] = item;
        fifo->wr = FIFO_NEXT(*fifo, fifo->wr);
        ret = 0;
    }

    return ret;
}


/*************************************************************************//**
 * Remove item from FIFO
 *
 * @param[in,out]    fifo    FIFO context
 * @param[in,out]    item    Storage for removed item
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
int fifo_get(fifo_t *fifo, void **item)
{
    int ret = -1;

    if (!FIFO_EMPTY(*fifo))
    {
        *item = fifo->data[fifo->rd];
        fifo->rd = FIFO_NEXT(*fifo, fifo->rd);
        ret = 0;
    }

    return ret;
}


/*****************************************************************************
 * End of file
 ****************************************************************************/
