#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "fifo_ops.h"

static int __init kernel_fifo_init(void) {
  const int init_size = 100;

  if (fifo_init(init_size) < 0) {
    pr_err("Failed to create kfifo\n");
    return -ENOMEM;
  }

  pr_info("Kernel fifo module loaded successfully\n");
  return 0;
}

static void __exit kernel_fifo_exit(void) {
  fifo_free();

  pr_info("Kernel fifo module unloaded\n");
}

module_init(kernel_fifo_init);
module_exit(kernel_fifo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("FIFO Module based on kfifo");
MODULE_VERSION("1.0");