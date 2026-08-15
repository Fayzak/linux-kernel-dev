#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "consumer.h"
#include "producer.h"

static unsigned int fifo_size = 4;
static unsigned int num_events = 200;
static unsigned int interval_us = 1000;

module_param(fifo_size, uint, 0644);
module_param(num_events, uint, 0644);
module_param(interval_us, uint, 0644);

MODULE_PARM_DESC(fifo_size, "kfifo size");
MODULE_PARM_DESC(num_events,
                 "The number of events that the producer will generate");
MODULE_PARM_DESC(interval_us,
                 "The interval between producer events in microseconds");

static int is_power_of_two(unsigned int n) {
  return n != 0 && (n & (n - 1)) == 0;
}

static int __init kernel_pc_demo_init(void) {
  if (fifo_size < 4 || fifo_size > 1024) {
    pr_err("Error: fifo_size must be >= 4 and <= 1024\n");
    return PC_INVALID;
  }
  if (!is_power_of_two(fifo_size)) {
    pr_err("Error: fifo_size must power of two\n");
    return PC_INVALID;
  }

  if (num_events < 1 || num_events > 50000) {
    pr_err("Error: num_events must be >= 1 and <= 50000\n");
    return PC_INVALID;
  }

  if (interval_us < 100 || interval_us > 1000000) {
    pr_err("Error: interval_us must be >= 100 and <= 1000000\n");
    return PC_INVALID;
  }

  struct pc_ctx *ctx = producergetctx();
  int res = producersetctx(ctx, fifo_size, num_events, interval_us);
  if (res < 0) {
    pr_err("Error: not enough memory for kfifo\n");
    return PC_NOMEM;
  }

  pr_info("kernel_pc_demo_module loaded successfully: fifo_size=%u "
          "num_events=%u interval_us=%u\n",
          fifo_size, num_events, interval_us);
  return 0;
}

static void __exit kernel_pc_demo_exit(void) {
  struct pc_ctx *ctx = producergetctx();
  producercleanup(ctx);
  pr_info("kernel_pc_demo_module unloaded\n");
}

module_init(kernel_pc_demo_init);
module_exit(kernel_pc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("Producer/Consumer Tasklet vs Workqueue Module");
MODULE_VERSION("1.0");
