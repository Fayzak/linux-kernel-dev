#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "sync.h"
#include "worker.h"

static unsigned int num_threads = 4;
static unsigned int iterations = 1000;

module_param(num_threads, uint, 0644);
module_param(iterations, uint, 0644);

MODULE_PARM_DESC(num_threads, "The number of kthread workers simultaneously "
                              "modifying the counter (1-32).");
MODULE_PARM_DESC(
    iterations,
    "Number of iterations (increment + decrement) per thread (1-1000000).");

static int __init kernel_sync_demo_init(void) {
  if (num_threads == 0 || num_threads > 32) {
    pr_err("Error: num_threads must be >= 1 and <= 32\n");
    return SD_INVALID;
  }

  if (iterations == 0 || iterations > 1000000) {
    pr_err("Error: iterations must be >= 1 and <= 1000000\n");
    return SD_INVALID;
  }

  struct sync_ctx *ctx = syncgetctx();
  syncctxinit(ctx, num_threads, iterations, 0);

  pr_info("kernel_sync_demo_module loaded successfully: num_threads=%u "
          "iterations=%u\n",
          num_threads, iterations);
  return 0;
}

static void __exit kernel_sync_demo_exit(void) {
  struct sync_ctx *ctx = syncgetctx();
  synccleanup(ctx);
  pr_info("kernel_sync_demo_module unloaded\n");
}

module_init(kernel_sync_demo_init);
module_exit(kernel_sync_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("Kthreads Sync Module");
MODULE_VERSION("1.0");
