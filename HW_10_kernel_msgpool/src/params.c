#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>

#include "queue.h"

extern unsigned int alloc_type;
extern unsigned int interval_ms;

static int send_set(const char *val, const struct kernel_param *kp) {
  struct msgpool_ctx *ctx = queuegetctx();
  int ret = allocsend(ctx, val);

  return ret;
}

static int inbox_get(char *buf, const struct kernel_param *kp) {
  struct msgpool_ctx *ctx = queuegetctx();
  queueinbox(ctx, buf);
  return strlen(buf);
}

static int stats_get(char *buf, const struct kernel_param *kp) {
  struct msgpool_ctx *ctx = queuegetctx();
  queuestats(ctx, buf);
  return strlen(buf);
}

static int flush_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;
  int ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("flush_set: kstrtouint error\n");
    return MP_INVALID;
  }
  if (x == 0)
    return MP_OK;

  struct msgpool_ctx *ctx = queuegetctx();
  queueflush(ctx);

  return MP_OK;
}

static int alloc_type_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;
  int ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("alloc_type_set: kstrtouint error\n");
    return MP_INVALID;
  }
  if (x != 0 && x != 1) {
    pr_err("Error: alloc_type must be 0 or 1\n");
    return MP_INVALID;
  }

  struct msgpool_ctx *ctx = queuegetctx();
  if (ctx->alloc_type == x)
    return MP_OK;
  if (ctx->alloc_type == 1 && x == 0) {
    if (ctx->msg_pool) {
      mempool_destroy(ctx->msg_pool);
      ctx->msg_pool = NULL;
    }
  } else if (ctx->alloc_type == 0 && x == 1) {
    ctx->msg_pool = mempool_create_slab_pool(ctx->pool_min_nr, ctx->msg_cache);
    if (!ctx->msg_pool)
      return MP_NOMEM;
  }
  ctx->alloc_type = x;
  alloc_type = x;

  return MP_OK;
}

static int alloc_type_get(char *buf, const struct kernel_param *kp) {
  const char *alloc_types[] = {"kmem_cache", "mempool"};

  snprintf(buf, PAGE_SIZE, "alloc_type=%s\n", alloc_types[alloc_type]);
  return strlen(buf);
}

static int interval_ms_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;
  int ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("interval_ms_set: kstrtouint error\n");
    return MP_INVALID;
  }
  if (x < 100 || x > 60000) {
    pr_err("Error: interval_ms must be >= 100 and <= 60000\n");
    return MP_INVALID;
  }

  struct msgpool_ctx *ctx = queuegetctx();
  ctx->interval_ms = x;
  interval_ms = x;
  mod_timer(&ctx->consumer_timer, jiffies + msecs_to_jiffies(interval_ms));

  return MP_OK;
}

static int interval_ms_get(char *buf, const struct kernel_param *kp) {
  snprintf(buf, PAGE_SIZE, "interval_ms=%d\n", interval_ms);
  return strlen(buf);
}

static const struct kernel_param_ops param_ops_send = {
    .set = send_set,
};
module_param_cb(send, &param_ops_send, NULL, 0200);
MODULE_PARM_DESC(send, "Send a message to the queue");

static const struct kernel_param_ops param_ops_inbox = {
    .get = inbox_get,
};
module_param_cb(inbox, &param_ops_inbox, NULL, 0444);
MODULE_PARM_DESC(inbox, "Read the last processed message (or \"empty\")");

static const struct kernel_param_ops param_ops_stats = {
    .get = stats_get,
};
module_param_cb(stats, &param_ops_stats, NULL, 0444);
MODULE_PARM_DESC(stats,
                 "Statistics: Sent / Processed / Flushed / Current Queue Size");

static const struct kernel_param_ops param_ops_flush = {
    .set = flush_set,
};
module_param_cb(flush, &param_ops_flush, NULL, 0200);
MODULE_PARM_DESC(flush, "Forcefully release all messages from the queue");

static const struct kernel_param_ops param_ops_alloc_type = {
    .set = alloc_type_set, .get = alloc_type_get};
module_param_cb(alloc_type, &param_ops_alloc_type, NULL, 0644);
MODULE_PARM_DESC(alloc_type, "Allocator type: 0 — kmem_cache, 1 — mempool");

static const struct kernel_param_ops param_ops_interval_ms = {
    .set = interval_ms_set, .get = interval_ms_get};
module_param_cb(interval_ms, &param_ops_interval_ms, NULL, 0644);
MODULE_PARM_DESC(interval_ms, "Consumer-timer response period (ms)");
