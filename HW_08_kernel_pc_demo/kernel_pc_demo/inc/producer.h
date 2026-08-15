#ifndef PRODUCER_H
#define PRODUCER_H

#include <linux/hrtimer_types.h>
#include <linux/interrupt.h>
#include <linux/kfifo.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue_types.h>

#define PC_OK 0               /* операция успешна */
#define PC_INVALID -EINVAL    /* неверный параметр */
#define PC_NOMEM -ENOMEM      /* недостаточно памяти */
#define PC_BUSY -EBUSY        /* тест уже выполняется */
#define PC_TIMEOUT -ETIMEDOUT /* тест не завершился вовремя */

struct pc_ctx {
  unsigned int fifo_size;
  unsigned int num_events;
  unsigned int interval_us;
  unsigned int consumer_type;

  /* Очередь */
  DECLARE_KFIFO_PTR(fifo, unsigned int);

  /* Producer */
  struct hrtimer timer;
  atomic_t produced; /* сколько событий положено в fifo */
  atomic_t dropped;  /* сколько событий дропнуто (fifo полон) */

  /* Consumer — tasklet */
  struct tasklet_struct tasklet;

  /* Consumer — workqueue */
  wait_queue_head_t waitq; /* Очередь ожидания для wait_event_timeout */
  struct workqueue_struct *wq;
  struct work_struct work;

  /* Статистика обработки */
  atomic_t consumed;       /* сколько событий обработано */
  u64 sum;                 /* сумма обработанных значений */
  unsigned int last_value; /* последнее обработанное значение */
  struct mutex stats_lock; /* защита sum и last_value (только для WQ) */

  int last_run_result;
};

int producersetctx(struct pc_ctx *ctx, unsigned int fifo_size,
                   unsigned int num_events, unsigned int interval_us);
struct pc_ctx *producergetctx(void);
void producercleanup(struct pc_ctx *ctx);
int producerrun(struct pc_ctx *ctx);
void producerresult(struct pc_ctx *ctx, char *str);
void producerstats(struct pc_ctx *ctx, char *str);
int producerconsumertype(struct pc_ctx *ctx, int type);
int producerreset(struct pc_ctx *ctx);

#endif
