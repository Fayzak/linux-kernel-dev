#ifndef SYNC_H
#define SYNC_H

#include <linux/atomic.h>
#include <linux/ktime.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#define SD_OK 0            /* операция успешна */
#define SD_INVALID -EINVAL /* неверный параметр */
#define SD_NOMEM -ENOMEM   /* недостаточно памяти */
#define SD_BUSY -EBUSY     /* тест уже выполняется */

struct sync_ctx {
  unsigned int num_threads;
  unsigned int iterations;
  unsigned int lock_type;

  long long shared_counter; /* защищаемый счётчик */

  spinlock_t slock;
  struct mutex mlock;
  struct semaphore sem;

  /* Статистика */
  ktime_t total_wait_time;       /* суммарное время ожидания блокировки */
  unsigned int contention_count; /* число раз, когда поток ждал */

  /* Управление потоками */
  struct task_struct **threads;
  atomic_t threads_done;
  int last_run_result; /* 0 = OK, <0 = ошибка */
};

struct sync_ctx *syncgetctx(void);
void syncctxinit(struct sync_ctx *ctx, unsigned int num_threads,
                 unsigned int iterations, unsigned int lock_type);
int syncrun(struct sync_ctx *ctx);
void syncresult(struct sync_ctx *ctx, unsigned char *str);
void syncstats(struct sync_ctx *ctx, unsigned char *str);
void syncreset(struct sync_ctx *ctx);
void synccleanup(struct sync_ctx *ctx);

#endif
