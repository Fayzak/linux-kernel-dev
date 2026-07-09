#include <linux/atomic.h>
#include <linux/kthread.h>
#include <linux/timekeeping.h>

#include "worker.h"

int workerwork(void *data) {
  struct worker_args *args = (struct worker_args *)data;
  struct sync_ctx *ctx = args->ctx;

  const ktime_t thr = 100;

  for (unsigned int i = 0; i < ctx->iterations && !kthread_should_stop(); i++) {
    ktime_t t_before = ktime_get();

    switch (ctx->lock_type) {
    case 0:
      spin_lock(&ctx->slock);
      break;
    case 1:
      mutex_lock(&ctx->mlock);
      break;
    case 2:
      down(&ctx->sem);
      break;
    }

    ktime_t t_after = ktime_get();
    ktime_t delta = ktime_sub(t_after, t_before);

    if (ktime_after(delta, thr)) {
      args->wait_time = ktime_add(args->wait_time, delta);
      ctx->contention_count++;
    }

    ctx->shared_counter++;

    switch (ctx->lock_type) {
    case 0:
      spin_unlock(&ctx->slock);
      break;
    case 1:
      mutex_unlock(&ctx->mlock);
      break;
    case 2:
      up(&ctx->sem);
      break;
    }

    switch (ctx->lock_type) {
    case 0:
      spin_lock(&ctx->slock);
      break;
    case 1:
      mutex_lock(&ctx->mlock);
      break;
    case 2:
      down(&ctx->sem);
      break;
    }

    ctx->shared_counter--;

    switch (ctx->lock_type) {
    case 0:
      spin_unlock(&ctx->slock);
      break;
    case 1:
      mutex_unlock(&ctx->mlock);
      break;
    case 2:
      up(&ctx->sem);
      break;
    }
  }

  atomic_inc(&ctx->threads_done);

  return 0;
}
