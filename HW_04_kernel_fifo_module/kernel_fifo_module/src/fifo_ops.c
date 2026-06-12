#include <linux/kfifo.h>

#include <fifo_ops.h>

#define FIFO_OK 0       /* Операция успешна */
#define FIFO_EMPTY -1   /* Очередь пуста */
#define FIFO_FULL -2    /* Очередь полна */
#define FIFO_NOMEM -3   /* Нет памяти */
#define FIFO_INVALID -4 /* Неверный параметр */

struct fifo_entry {
  int data; /* Данные (целое число) */
};

struct fifo_device {
  DECLARE_KFIFO_PTR(queue, struct fifo_entry); /* Встроенный буфер kfifo */
  int max_size;                                /* Максимальный размер очереди */
};

static struct fifo_device kfifo_fifo;

int fifo_init(int size) {
  if (size <= 0) {
    return FIFO_INVALID;
  }

  int ret = kfifo_alloc(&kfifo_fifo.queue, size, GFP_KERNEL);
  if (ret) {
    return FIFO_NOMEM;
  }

  kfifo_fifo.max_size = size;
  return FIFO_OK;
}

int fifo_enqueue(int value) {
  struct fifo_entry entry;
  entry.data = value;

  if (!kfifo_put(&kfifo_fifo.queue, entry)) {
    return FIFO_FULL;
  }

  return FIFO_OK;
}

int fifo_dequeue(void) {
  struct fifo_entry entry;

  if (kfifo_get(&kfifo_fifo.queue, &entry)) {
    return entry.data;
  }

  return FIFO_EMPTY;
}

int fifo_peek(void) {
  struct fifo_entry entry;

  if (kfifo_peek(&kfifo_fifo.queue, &entry)) {
    return entry.data;
  }

  return FIFO_EMPTY;
}

int fifo_is_empty(void) { return kfifo_is_empty(&kfifo_fifo.queue); }

int fifo_is_full(void) { return kfifo_is_full(&kfifo_fifo.queue); }

int fifo_size(void) { return kfifo_len(&kfifo_fifo.queue); }

int fifo_available(void) { return kfifo_avail(&kfifo_fifo.queue); }

void fifo_clear(void) { kfifo_reset(&kfifo_fifo.queue); }

void fifo_free(void) { kfifo_free(&kfifo_fifo.queue); }
