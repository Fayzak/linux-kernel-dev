#include <linux/sysfs.h>

#include "kernel_stack.h"

static ssize_t push_store(struct kobject *kobj, struct kobj_attribute *attr,
                          const char *buf, size_t count) {
  int value;
  if (kstrtoint(buf, 10, &value) != 0)
    return STACK_INVALID;

  if (stack_push(value) < 0)
    return STACK_NOMEM;

  return count;
}

static ssize_t pop_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf) {
  int ret;
  ret = stack_pop();

  if (ret >= 0)
    return sysfs_emit(buf, "%d\n", ret);
  else
    return sysfs_emit(buf, "Error: stack is empty\n");
}

static ssize_t peek_show(struct kobject *kobj, struct kobj_attribute *attr,
                         char *buf) {
  int ret;
  ret = stack_peek();

  if (ret >= 0)
    return sysfs_emit(buf, "%d\n", ret);
  else
    return sysfs_emit(buf, "Error: stack is empty\n");
}

static ssize_t size_show(struct kobject *kobj, struct kobj_attribute *attr,
                         char *buf) {
  int size;
  size = stack_size();

  return sysfs_emit(buf, "%d\n", size);
}

static ssize_t is_empty_show(struct kobject *kobj, struct kobj_attribute *attr,
                             char *buf) {
  int empty;
  empty = stack_is_empty();

  return sysfs_emit(buf, "%d\n", empty ? 1 : 0);
}

static ssize_t clear_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count) {
  stack_clear();

  return count;
}

static struct kobj_attribute push_attr = __ATTR_WO(push);
static struct kobj_attribute pop_attr = __ATTR_RO(pop);
static struct kobj_attribute peek_attr = __ATTR_RO(peek);
static struct kobj_attribute size_attr = __ATTR_RO(size);
static struct kobj_attribute is_empty_attr = __ATTR_RO(is_empty);
static struct kobj_attribute clear_attr = __ATTR_WO(clear);

static struct attribute *stack_attrs[] = {
    &push_attr.attr,
    &pop_attr.attr,
    &peek_attr.attr,
    &size_attr.attr,
    &is_empty_attr.attr,
    &clear_attr.attr,
    NULL,
};

static struct attribute_group stack_attr_group = {
    .attrs = stack_attrs,
};

int stack_sysfs_init(void) {
  return sysfs_create_group(kernel_stack_kobj, &stack_attr_group);
}

void stack_sysfs_exit(void) {
  sysfs_remove_group(kernel_stack_kobj, &stack_attr_group);
}