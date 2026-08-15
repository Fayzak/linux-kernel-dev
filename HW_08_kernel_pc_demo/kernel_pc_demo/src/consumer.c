#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>
#include <linux/types.h>

#include "producer.h"

#include "consumer.h"

void tasklet_consumer(unsigned long data) {
  struct pc_ctx *ctx = (struct pc_ctx *)data;
  unsigned int val;

  while (kfifo_get(&ctx->fifo, &val)) {
    ctx->sum += val;
    ctx->last_value = val;
    atomic_inc(&ctx->consumed);
  }
}

void work_consumer(struct work_struct *work) {
  struct pc_ctx *ctx = container_of(work, struct pc_ctx, work);
  unsigned int val;

  while (1) {
    if (kfifo_get(&ctx->fifo, &val)) {
      mutex_lock(&ctx->stats_lock);
      ctx->sum += val;
      ctx->last_value = val;
      mutex_unlock(&ctx->stats_lock);
      atomic_inc(&ctx->consumed);
    } else {
      /* fifo пуст — если producer ещё работает, подождать */
      int total = atomic_read(&ctx->produced) + atomic_read(&ctx->dropped);
      if (total < ctx->num_events) {
        msleep(1);
        continue;
      }
      break;
    }
  }
}
