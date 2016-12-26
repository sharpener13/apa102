#include <stdio.h>
#include <malloc.h>
#include "fifo.h"


void fifo_init(fifo_t *fifo, int size)
{
    fifo->size   = size;
    fifo->data   = (void **)malloc((size + 1) * sizeof(void *));
    fifo->rd     = fifo->wr = 0;
}


void fifo_done(fifo_t *fifo)
{
    free(fifo->data);
    fifo->data = NULL;
    fifo->size = 0;
}


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

