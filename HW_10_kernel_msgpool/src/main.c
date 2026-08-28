#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/timer.h>

#include "queue.h"

unsigned int alloc_type = 0;
unsigned int interval_ms = 1000;
static unsigned int pool_min_nr = 8;

module_param(pool_min_nr, uint, 0644);
MODULE_PARM_DESC(
    pool_min_nr,
    "Minimum reserve of objects in the mempool (used when alloc_type=1)");

static void timer_callback(struct timer_list *t) {
  struct msgpool_ctx *ctx = queuegetctx();
  struct msg *local_objs[MSG_QUEUE_MAX];
  int count = 0;

  spin_lock(&ctx->queue.lock);
  while (ctx->queue.count > 0) {
    local_objs[count++] = ctx->queue.slots[ctx->queue.head];
    ctx->queue.head = (ctx->queue.head + 1) % MSG_QUEUE_MAX;
    ctx->queue.count--;
  }
  spin_unlock(&ctx->queue.lock);

  for (int i = 0; i < count; i++) {
    struct msg *obj = local_objs[i];
    s64 elapsed_ns = ktime_to_ns(ktime_sub(ktime_get(), obj->enqueue_time));

    pr_info("msgpool: [^%u] %s (queued %lld ns ago)\n", obj->seq, obj->text,
            elapsed_ns);

    spin_lock(&ctx->last_msg_lock);
    strscpy(ctx->last_msg, obj->text, MSG_TEXT_MAX);
    spin_unlock(&ctx->last_msg_lock);

    if (ctx->alloc_type == 0)
      kmem_cache_free(ctx->msg_cache, obj);
    else
      mempool_free(obj, ctx->msg_pool);

    atomic_inc(&ctx->consumed_total);
  }

  mod_timer(&ctx->consumer_timer, jiffies + msecs_to_jiffies(ctx->interval_ms));
}

static int __init kernel_msgpoll_init(void) {
  if (alloc_type != 0 && alloc_type != 1) {
    pr_err("Error: alloc_type must be 0 or 1\n");
    return MP_INVALID;
  }

  if (pool_min_nr < 1 || pool_min_nr > 64) {
    pr_err("Error: pool_min_nr must be >= 1 and <= 64\n");
    return MP_INVALID;
  }

  if (interval_ms < 100 || interval_ms > 60000) {
    pr_err("Error: interval_ms must be >= 100 and <= 60000\n");
    return MP_INVALID;
  }

  struct msgpool_ctx *ctx = queuegetctx();
  queuesetctx(ctx, alloc_type, pool_min_nr, interval_ms);
  timer_setup(&ctx->consumer_timer, timer_callback, 0);
  mod_timer(&ctx->consumer_timer, jiffies + msecs_to_jiffies(interval_ms));

  pr_info("kernel_msgpoll_module loaded successfully: alloc_type=%u "
          "pool_min_nr=%u interval_ms=%u\n",
          alloc_type, pool_min_nr, interval_ms);
  return 0;
}

static void __exit kernel_msgpoll_exit(void) {
  struct msgpool_ctx *ctx = queuegetctx();
  queueclearctx(ctx);

  pr_info("kernel_msgpoll_module unloaded\n");
}

module_init(kernel_msgpoll_init);
module_exit(kernel_msgpoll_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("Message Poll based on kmem_cahce and mempool Module");
MODULE_VERSION("1.0");
