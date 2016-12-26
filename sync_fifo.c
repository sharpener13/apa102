/*************************************************************************//**
 * @file sync_fifo.c
 *
 *     Synchronization layer for simple FIFO handling
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "fifo.h"
#include "sync_fifo.h"


/*******************************************************************************
 * Public functions
 ******************************************************************************/


/*************************************************************************//**
 * Initialize FIFO
 *
 * @param[in,out]    fifo    FIFO context
 * @param[in]        size    Number of items the FIFO can hold
 * @param[in]        name    FIFO name (not used for anything)
 *
 ****************************************************************************/
void sync_fifo_init(sync_fifo_t *fifo, int size, char *name)
{
    fifo->name = name;
    fifo_init(&fifo->raw, size);
    pthread_mutex_init(&fifo->mx, NULL);
    pthread_cond_init(&fifo->cv_ne, NULL);
    pthread_cond_init(&fifo->cv_nf, NULL);
}


/*************************************************************************//**
 * Finalize FIFO
 *
 * @param[in,out]    fifo    FIFO context
 *
 ****************************************************************************/
void sync_fifo_done(sync_fifo_t *fifo)
{
    fifo_done(&fifo->raw);
    pthread_mutex_destroy(&fifo->mx);
    pthread_cond_destroy(&fifo->cv_ne);
    pthread_cond_destroy(&fifo->cv_nf);
}


/*************************************************************************//**
 * Insert item into FIFO
 *
 * @param[in,out]    fifo          FIFO context
 * @param[in]        item          Item to be inserted
 * @param[in]        is_waiting    Wait until FIFO is not full
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
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


/*************************************************************************//**
 * Remove item from FIFO
 *
 * @param[in,out]    fifo          FIFO context
 * @param[in,out]    item          Storage for removed item
 * @param[in]        is_waiting    Wait until FIFO is not empty 
 *
 * @return    zero on success, nonzero otherwise
 *
 ****************************************************************************/
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


/*****************************************************************************
 * End of file
 ****************************************************************************/
