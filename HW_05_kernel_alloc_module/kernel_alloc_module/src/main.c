#include <linux/init.h>
#include <linux/module.h>

#include "allocator.h"

static int __init kernel_alloc_init(void) {
  int ret;
  ret = allocator_init();
  if (ret != ALLOC_OK) {
    pr_err("kernel_alloc_module failed to initialize allocator (%d)\n", ret);
    return ret;
  }

  pr_info("kernel_alloc_module loaded successfully\n");
  return 0;
}

static void __exit kernel_alloc_exit(void) {
  allocator_cleanup();
  pr_info("kernel_alloc_module unloaded\n");
}

module_init(kernel_alloc_init);
module_exit(kernel_alloc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("Alloc Module based on bitmap");
MODULE_VERSION("1.0");
