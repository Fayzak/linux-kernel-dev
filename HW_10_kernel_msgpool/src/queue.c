#include <linux/string.h>

#include "queue.h"

static struct msgpool_ctx ctx;

struct msgpool_ctx *queuegetctx(void) { return &ctx; }

int queuesetctx(struct msgpool_ctx *ctx, unsigned int alloc_type,
                unsigned int pool_min_nr, unsigned int interval_ms) {
  ctx->alloc_type = alloc_type;
  ctx->pool_min_nr = pool_min_nr;
  ctx->interval_ms = interval_ms;

  spin_lock_init(&ctx->queue.lock);
  spin_lock_init(&ctx->last_msg_lock);

  atomic_set(&ctx->sent_total, 0);
  atomic_set(&ctx->consumed_total, 0);
  atomic_set(&ctx->flushed_total, 0);
  atomic_set(&ctx->dropped_total, 0);
  ctx->seq_counter = 0;
  strscpy(ctx->last_msg, "empty", MSG_TEXT_MAX);

  ctx->msg_cache =
      kmem_cache_create("msgpool_cache", sizeof(struct msg), 0, 0, NULL);
  if (!ctx->msg_cache)
    return MP_NOMEM;

  ctx->msg_pool = NULL;
  if (alloc_type == 1) {
    ctx->msg_pool = mempool_create_slab_pool(pool_min_nr, ctx->msg_cache);
    if (!ctx->msg_pool) {
      kmem_cache_destroy(ctx->msg_cache);
      return MP_NOMEM;
    }
  }

  return MP_OK;
}

void queueclearctx(struct msgpool_ctx *ctx) {
  del_timer_sync(&ctx->consumer_timer);
  queueflush(ctx);

  if (ctx->alloc_type == 1 && ctx->msg_pool)
    mempool_destroy(ctx->msg_pool);
  kmem_cache_destroy(ctx->msg_cache);
}

void queueflush(struct msgpool_ctx *ctx) {
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
    if (ctx->alloc_type == 0)
      kmem_cache_free(ctx->msg_cache, local_objs[i]);
    else
      mempool_free(local_objs[i], ctx->msg_pool);
  }

  if (count > 0)
    atomic_add(count, &ctx->flushed_total);
}

void queueinbox(struct msgpool_ctx *ctx, char *str) {
  spin_lock(&ctx->last_msg_lock);
  snprintf(str, PAGE_SIZE, "%s\n", ctx->last_msg);
  spin_unlock(&ctx->last_msg_lock);
}

void queuestats(struct msgpool_ctx *ctx, char *str) {
  const char *alloc_types[] = {"kmem_cache", "mempool"};

  spin_lock(&ctx->queue.lock);
  snprintf(str, PAGE_SIZE,
           "sent=%d consumed=%d flushed=%d dropped=%d queued=%d alloc=%s "
           "interval_ms=%d\n",
           atomic_read(&ctx->sent_total), atomic_read(&ctx->consumed_total),
           atomic_read(&ctx->flushed_total), atomic_read(&ctx->dropped_total),
           ctx->queue.count, alloc_types[ctx->alloc_type], ctx->interval_ms);
  spin_unlock(&ctx->queue.lock);
}
