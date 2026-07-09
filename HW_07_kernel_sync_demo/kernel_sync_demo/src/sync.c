#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>

#include "worker.h"

#include "sync.h"

static struct sync_ctx ctx;

struct sync_ctx *syncgetctx(void) { return &ctx; }

void syncctxinit(struct sync_ctx *ctx, unsigned int num_threads,
                 unsigned int iterations, unsigned int lock_type) {
  ctx->num_threads = num_threads;
  ctx->iterations = iterations;
  ctx->lock_type = lock_type;

  ctx->shared_counter = 0;
  ctx->total_wait_time = 0;
  ctx->last_run_result = 0;
  ctx->contention_count = 0;
  atomic_set(&ctx->threads_done, 0);

  ctx->threads =
      kmalloc_array(num_threads, sizeof(struct task_struct *), GFP_KERNEL);
  if (ctx->threads) {
    for (int i = 0; i < num_threads; i++) {
      ctx->threads[i] = NULL;
    }
  }
}

int syncrun(struct sync_ctx *ctx) {
  if (ctx->last_run_result > 0)
    return SD_BUSY;

  ctx->last_run_result = 1;

  switch (ctx->lock_type) {
  case 0:
    spin_lock_init(&ctx->slock);
    break;
  case 1:
    mutex_init(&ctx->mlock);
    break;
  case 2:
    sema_init(&ctx->sem, 1);
    break;
  }

  ctx->shared_counter = 0;
  ctx->total_wait_time = 0;
  ctx->contention_count = 0;
  atomic_set(&ctx->threads_done, 0);

  struct worker_args *args =
      kmalloc_array(ctx->num_threads, sizeof(struct worker_args), GFP_KERNEL);
  if (!args) {
    ctx->last_run_result = SD_NOMEM;
    return SD_NOMEM;
  }

  for (int i = 0; i < ctx->num_threads; i++) {
    args[i].ctx = ctx;
    args[i].thread_id = i;
    args[i].wait_time = 0;

    ctx->threads[i] = kthread_create(workerwork, &args[i], "worker_%u", i);
    if (IS_ERR(ctx->threads[i])) {
      ctx->last_run_result = PTR_ERR(ctx->threads[i]);
      kfree(args);
      return PTR_ERR(ctx->threads[i]);
    }
    wake_up_process(ctx->threads[i]);
  }

  for (int i = 0; i < ctx->num_threads; i++) {
    kthread_stop(ctx->threads[i]);
  }

  for (int i = 0; i < ctx->num_threads; i++) {
    ctx->total_wait_time = ktime_add(ctx->total_wait_time, args[i].wait_time);
  }

  kfree(args);

  ctx->last_run_result = 0;
  return SD_OK;
}

void syncresult(struct sync_ctx *ctx, unsigned char *str) {
  const char *lock_types[] = {"spinlock", "mutex", "semaphore"};
  const char *status =
      (ctx->shared_counter == 0 && ctx->last_run_result == 0) ? "ok" : "fail";

  snprintf(str, PAGE_SIZE, "counter=%lld threads=%u iterations=%u lock=%s %s\n",
           ctx->shared_counter, ctx->num_threads, ctx->iterations,
           lock_types[ctx->lock_type], status);
}

void syncstats(struct sync_ctx *ctx, unsigned char *str) {
  unsigned int contention = ctx->contention_count;
  long long total_ns = ktime_to_ns(ctx->total_wait_time);
  long long avg_ns = 0;

  if (contention > 0) {
    avg_ns = total_ns / contention;
  }

  snprintf(str, PAGE_SIZE,
           "contention=%u total_wait_ns=%lld avg_wait_ns=%lld\n", contention,
           total_ns, avg_ns);
}

void syncreset(struct sync_ctx *ctx) {
  if (ctx->last_run_result > 0) {
    return;
  }

  ctx->shared_counter = 0;
  ctx->total_wait_time = 0;
  ctx->contention_count = 0;
  ctx->last_run_result = 0;
}

void synccleanup(struct sync_ctx *ctx) {
  if (ctx->threads) {
    kfree(ctx->threads);
    ctx->threads = NULL;
  }

  ctx->shared_counter = 0;
  ctx->total_wait_time = 0;
  ctx->contention_count = 0;
  atomic_set(&ctx->threads_done, 0);
}
