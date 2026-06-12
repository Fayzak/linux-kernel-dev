#ifndef FIFO_OPS_H
#define FIFO_OPS_H

int fifo_init(int size);
int fifo_enqueue(int value);
int fifo_dequeue(void);
int fifo_peek(void);
int fifo_is_empty(void);
int fifo_is_full(void);
int fifo_size(void);
int fifo_available(void);
void fifo_clear(void);
void fifo_free(void);

#endif