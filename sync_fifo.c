#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "fifo.h"
#include "sync_fifo.h"


void sync_fifo_init(sync_fifo_t *fifo, int size, char *name)
{
    fifo->name = name;
    fifo_init(&fifo->raw, size);
    pthread_mutex_init(&fifo->mx, NULL);
    pthread_cond_init(&fifo->cv_ne, NULL);
    pthread_cond_init(&fifo->cv_nf, NULL);
}


void sync_fifo_done(sync_fifo_t *fifo)
{
    fifo_done(&fifo->raw);
    pthread_mutex_destroy(&fifo->mx);
    pthread_cond_destroy(&fifo->cv_ne);
    pthread_cond_destroy(&fifo->cv_nf);
}


int sync_fifo_put(sync_fifo_t *fifo, void *item, bool is_waiting)
{
    int ret = 0;

    pthread_mutex_lock(&fifo->mx);
    {
        while (FIFO_FULL(fifo->raw))
        {
            if (is_waiting)
                pthread_cond_wait(&fifo->cv_nf, &fifo->mx);
            else
            {
                pthread_mutex_unlock(&fifo->mx);
                return -1;
            }
        }

        ret = fifo_put(&fifo->raw, item);
        pthread_mutex_unlock(&fifo->mx);
    }
    pthread_cond_signal(&fifo->cv_ne);

    return ret;
}


int sync_fifo_get(sync_fifo_t *fifo, void **item, bool is_waiting)
{
    int ret = 0;

    pthread_mutex_lock(&fifo->mx);
    {
        while (FIFO_EMPTY(fifo->raw))
        {
            if (is_waiting)
                pthread_cond_wait(&fifo->cv_ne, &fifo->mx);
            else
            {
                pthread_mutex_unlock(&fifo->mx);
                return -1;
            }
        }

        ret = fifo_get(&fifo->raw, item);
        pthread_mutex_unlock(&fifo->mx);
    }
    pthread_cond_signal(&fifo->cv_nf);

    return ret;
}
