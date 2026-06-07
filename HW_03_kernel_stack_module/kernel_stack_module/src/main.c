#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>

#include "kernel_stack.h"

struct kobject *kernel_stack_kobj;

static int __init kernel_stack_init(void) {
  int ret;

  stack_init();

  kernel_stack_kobj = kobject_create_and_add("kernel_stack", kernel_kobj);
  if (!kernel_stack_kobj) {
    pr_err("Failed to crate kobject\n");
    return -ENOMEM;
  }

  ret = stack_sysfs_init();
  if (ret) {
    pr_err("Failed to create sysfs attributes\n");
    kobject_put(kernel_stack_kobj);
    return ret;
  }

  pr_info("Kernel stack module loaded successfully\n");
  return 0;
}

static void __exit kernel_stack_exit(void) {
  stack_sysfs_exit();

  stack_clear();

  kobject_put(kernel_stack_kobj);

  pr_info("Kernel stack module unloaded\n");
}

module_init(kernel_stack_init);
module_exit(kernel_stack_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("Kernel Stack Module based on head_list");
MODULE_VERSION("1.0");