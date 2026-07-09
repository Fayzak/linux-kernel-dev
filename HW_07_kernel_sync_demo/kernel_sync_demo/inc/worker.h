#ifndef WORKER_H
#define WORKER_H

#include "sync.h"

struct worker_args {
  struct sync_ctx *ctx;
  unsigned int thread_id;
  ktime_t wait_time; /* время ожидания конкретного потока */
};

int workerwork(void *data);

#endif
