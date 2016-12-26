#ifndef __SYNC_FIFO_H__
#define __SYNC_FIFO_H__

#include <stdbool.h>
#include <pthread.h>
#include "fifo.h"


typedef struct sync_fifo_tt
{
    fifo_t           raw;
    pthread_mutex_t  mx;
    pthread_cond_t   cv_ne;
    pthread_cond_t   cv_nf;
    char            *name;
} sync_fifo_t;


void sync_fifo_init(sync_fifo_t *fifo, int size, char *name);
void sync_fifo_done(sync_fifo_t *fifo);
int  sync_fifo_put (sync_fifo_t *fifo, void *item, bool is_waiting);
int  sync_fifo_get (sync_fifo_t *fifo, void **item, bool is_waiting);


#endif
