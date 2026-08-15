#include <linux/random.h>
#include <linux/wait.h>

#include "consumer.h"

#include "producer.h"

static struct pc_ctx g_ctx;

static enum hrtimer_restart producer_timer_callback(struct hrtimer *timer) {
  struct pc_ctx *ctx = container_of(timer, struct pc_ctx, timer);

  if (atomic_read(&ctx->produced) + atomic_read(&ctx->dropped) >=
      ctx->num_events) {
    wake_up(&ctx->waitq);
    return HRTIMER_NORESTART;
  }

  u32 val = get_random_u32() % 1000;

  if (!kfifo_put(&ctx->fifo, val)) {
    atomic_inc(&ctx->dropped); /* fifo полон — дроп */
  } else {
    atomic_inc(&ctx->produced);

    if (ctx->consumer_type == 0)
      tasklet_schedule(&ctx->tasklet);
    else
      queue_work(ctx->wq, &ctx->work);
  }

  hrtimer_forward_now(timer, ns_to_ktime(ctx->interval_us * 1000ULL));
  return HRTIMER_RESTART;
}

int producersetctx(struct pc_ctx *ctx, unsigned int fifo_size,
                   unsigned int num_events, unsigned int interval_us) {
  int ret;

  ctx->fifo_size = fifo_size;
  ctx->num_events = num_events;
  ctx->interval_us = interval_us;
  ctx->consumer_type = 0;

  atomic_set(&ctx->produced, 0);
  atomic_set(&ctx->dropped, 0);
  atomic_set(&ctx->consumed, 0);
  ctx->sum = 0;
  ctx->last_value = 0;
  ctx->last_run_result = 0;

  init_waitqueue_head(&ctx->waitq);

  ret = kfifo_alloc(&ctx->fifo, ctx->fifo_size, GFP_KERNEL);
  if (ret)
    return ret;

  hrtimer_init(&ctx->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  ctx->timer.function = producer_timer_callback;

  return 0;
}

struct pc_ctx *producergetctx(void) { return &g_ctx; }

void producercleanup(struct pc_ctx *ctx) {
  hrtimer_cancel(&ctx->timer);

  if (ctx->consumer_type == 0)
    tasklet_kill(&ctx->tasklet);
  else {
    if (ctx->wq) {
      flush_workqueue(ctx->wq);
      destroy_workqueue(ctx->wq);
      ctx->wq = NULL;
    }
  }

  kfifo_free(&ctx->fifo);
}

int producerrun(struct pc_ctx *ctx) {
  if (hrtimer_active(&ctx->timer))
    return PC_BUSY;

  kfifo_reset(&ctx->fifo);
  atomic_set(&ctx->produced, 0);
  atomic_set(&ctx->dropped, 0);
  atomic_set(&ctx->consumed, 0);
  ctx->sum = 0;
  ctx->last_value = 0;

  if (ctx->consumer_type == 0) {
    tasklet_init(&ctx->tasklet, tasklet_consumer, (unsigned long)ctx);
  } else {
    ctx->wq = alloc_workqueue("pc_demo_wq", WQ_UNBOUND | WQ_MEM_RECLAIM, 1);
    if (!ctx->wq)
      return PC_NOMEM;
    INIT_WORK(&ctx->work, work_consumer);
  }

  hrtimer_start(&ctx->timer, ns_to_ktime(ctx->interval_us * 1000ULL),
                HRTIMER_MODE_REL);

  /* Таймаут: время на все события + 2 секунды запаса */
  long timeout_j =
      usecs_to_jiffies((unsigned long)ctx->num_events * ctx->interval_us) +
      msecs_to_jiffies(2000);
  long ret = wait_event_timeout(
      ctx->waitq,
      (atomic_read(&ctx->produced) + atomic_read(&ctx->dropped) >=
       ctx->num_events),
      timeout_j);

  if (ret == 0) {
    hrtimer_cancel(&ctx->timer);
    ctx->last_run_result = PC_TIMEOUT;
    return PC_TIMEOUT;
  }

  ctx->last_run_result = PC_OK;
  return PC_OK;
}

void producerresult(struct pc_ctx *ctx, char *str) {
  const char *consumer_types[] = {"tasklet", "workqueue"};
  const char *status = ctx->last_run_result == PC_OK ? "ok" : "fail";

  if (atomic_read(&ctx->consumed) < atomic_read(&ctx->produced)) {
    snprintf(
        str, PAGE_SIZE,
        "produced=%d consumed=%d dropped=%d consumer=%s %s warn: lost=%d\n",
        atomic_read(&ctx->produced), atomic_read(&ctx->consumed),
        atomic_read(&ctx->dropped), consumer_types[ctx->consumer_type], status,
        atomic_read(&ctx->produced) - atomic_read(&ctx->consumed));
  } else {
    snprintf(
        str, PAGE_SIZE, "produced=%d consumed=%d dropped=%d consumer=%s %s\n",
        atomic_read(&ctx->produced), atomic_read(&ctx->consumed),
        atomic_read(&ctx->dropped), consumer_types[ctx->consumer_type], status);
  }
}

void producerstats(struct pc_ctx *ctx, char *str) {
  unsigned int consumed = atomic_read(&ctx->consumed);
  unsigned long long avg = (consumed > 0) ? (ctx->sum / consumed) : 0;

  snprintf(str, PAGE_SIZE,
           "produced=%d consumed=%d dropped=%d sum=%llu last=%u avg=%llu\n",
           atomic_read(&ctx->produced), consumed, atomic_read(&ctx->dropped),
           (unsigned long long)ctx->sum, ctx->last_value, avg);
}

int producerconsumertype(struct pc_ctx *ctx, int type) {
  if (hrtimer_active(&ctx->timer))
    return PC_BUSY;

  if (type < 0 || type > 1)
    return PC_INVALID;

  ctx->consumer_type = type;

  return PC_OK;
}

int producerreset(struct pc_ctx *ctx) {
  if (hrtimer_active(&ctx->timer))
    return PC_BUSY;

  hrtimer_cancel(&ctx->timer);

  if (ctx->consumer_type == 0) {
    tasklet_kill(&ctx->tasklet);
  } else {
    if (ctx->wq) { /* Защита от NULL */
      flush_workqueue(ctx->wq);
      destroy_workqueue(ctx->wq);
      ctx->wq = NULL;
    }
  }

  kfifo_reset(&ctx->fifo);

  atomic_set(&ctx->produced, 0);
  atomic_set(&ctx->dropped, 0);
  atomic_set(&ctx->consumed, 0);
  ctx->sum = 0;
  ctx->last_value = 0;

  ctx->last_run_result = 0;

  return PC_OK;
}
