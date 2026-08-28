#include <linux/string.h>

#include "queue.h"

int allocsend(struct msgpool_ctx *ctx, const char *txt) {
  if (strlen(txt) >= MSG_TEXT_MAX) {
    pr_warn("msgpool: message too long, truncating\n");
  }

  struct msg *obj = NULL;
  if (ctx->alloc_type == 0)
    obj = kmem_cache_alloc(ctx->msg_cache, GFP_KERNEL);
  else
    obj = mempool_alloc(ctx->msg_pool, GFP_KERNEL);

  if (!obj)
    return MP_NOMEM;

  strscpy(obj->text, txt, MSG_TEXT_MAX);
  obj->enqueue_time = ktime_get();
  obj->seq = ctx->seq_counter++;

  spin_lock(&ctx->queue.lock);
  if (ctx->queue.count < MSG_QUEUE_MAX) {
    ctx->queue.slots[ctx->queue.tail] = obj;
    ctx->queue.tail = (ctx->queue.tail + 1) % MSG_QUEUE_MAX;
    ctx->queue.count++;
    spin_unlock(&ctx->queue.lock);

    atomic_inc(&ctx->sent_total);
    return MP_OK;
  }
  spin_unlock(&ctx->queue.lock);

  if (ctx->alloc_type == 0)
    kmem_cache_free(ctx->msg_cache, obj);
  else
    mempool_free(obj, ctx->msg_pool);

  atomic_inc(&ctx->dropped_total);
  return MP_FULL;
}
