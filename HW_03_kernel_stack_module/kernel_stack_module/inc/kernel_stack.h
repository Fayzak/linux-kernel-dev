#ifndef KERNELSTACKH
#define KERNELSTACKH

#include <linux/kobject.h>

#include "stack.h"
#include "stack_ops.h"

extern struct kobject *kernel_stack_kobj;

int stack_sysfs_init(void);
void stack_sysfs_exit(void);

#endif