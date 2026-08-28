#ifndef QUEUE_H
#define QUEUE_H

#include <linux/atomic.h>
#include <linux/ktime.h>
#include <linux/mempool.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define MSG_TEXT_MAX 128
#define MSG_QUEUE_MAX 16

#define MP_OK 0              /* операция успешна             */
#define MP_INVALID (-EINVAL) /* неверный параметр            */
#define MP_NOMEM (-ENOMEM)   /* недостаточно памяти          */
#define MP_BUSY (-EBUSY)     /* операция недоступна сейчас   */
#define MP_FULL (-ENOBUFS)   /* очередь переполнена          */

struct msg {
  char text[MSG_TEXT_MAX]; /* текст сообщения                  */
  ktime_t enqueue_time;    /* момент постановки в очередь      */
  unsigned int seq;        /* порядковый номер сообщения        */
};

struct msg_queue {
  struct msg *slots[MSG_QUEUE_MAX]; /* слоты очереди (кольцевой буфер) */
  unsigned int head;                /* индекс чтения                   */
  unsigned int tail;                /* индекс записи                   */
  unsigned int count;               /* текущее число элементов         */
  spinlock_t lock;                  /* защита head/tail/count          */
};

struct msgpool_ctx {
  unsigned int alloc_type;
  unsigned int pool_min_nr;
  unsigned int interval_ms;

  struct kmem_cache *msg_cache; /* slab-кеш объектов struct msg       */
  mempool_t *msg_pool;          /* пул поверх msg_cache (alloc_type=1) */

  struct msg_queue queue;

  struct timer_list consumer_timer;

  /* Статистика */
  atomic_t sent_total;     /* всего поставлено в очередь         */
  atomic_t consumed_total; /* всего обработано таймером          */
  atomic_t flushed_total;  /* всего сброшено через flush         */
  atomic_t dropped_total;  /* отброшено (очередь была полна)     */

  /* Последнее обработанное сообщение (для параметра inbox) */
  char last_msg[MSG_TEXT_MAX];
  spinlock_t last_msg_lock;

  unsigned int seq_counter; /* монотонный счётчик сообщений       */
};

struct msgpool_ctx *queuegetctx(void);
int queuesetctx(struct msgpool_ctx *ctx, unsigned int alloc_type,
                unsigned int pool_min_nr, unsigned int interval_ms);
void queueclearctx(struct msgpool_ctx *ctx);
void queueflush(struct msgpool_ctx *ctx);
void queueinbox(struct msgpool_ctx *ctx, char *str);
void queuestats(struct msgpool_ctx *ctx, char *str);
int allocsend(struct msgpool_ctx *ctx, const char *txt);

#endif
