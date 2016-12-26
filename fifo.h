#ifndef __FIFO_H__
#define __FIFO_H__


#define FIFO_NEXT(f, i)  ((i + 1) % (((f).size) + 1))
#define FIFO_EMPTY(f)    ((f).rd == (f).wr)
#define FIFO_FULL(f)     (FIFO_NEXT(f, (f).wr) == (f).rd)


typedef struct fifo_tt
{
    int    size;
    void **data;
    int    rd;
    int    wr;
} fifo_t;


void fifo_init(fifo_t *fifo, int size);
void fifo_done(fifo_t *fifo);
int  fifo_put (fifo_t *fifo, void *item);
int  fifo_get (fifo_t *fifo, void **item);


#endif
