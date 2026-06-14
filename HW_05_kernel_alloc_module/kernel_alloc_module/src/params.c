#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "allocator.h"

static int alloc_set(const char *val, const struct kernel_param *kp) {
  int bytes;

  int ret;
  ret = kstrtoint(val, 10, &bytes);
  if (ret) {
    pr_err("alloc_set: kstrtoint error\n");
    return -EINVAL;
  }

  void *ptr = allocator_alloc((size_t)bytes);
  if (!ptr) {
    pr_err("alloc_set: allocation of %d bytes failed\n", bytes);
    return -ENOMEM;
  }

  pr_info("alloc_set: allocated %d bytes at 0x%px\n", bytes, ptr);
  return 0;
}

static int free_set(const char *val, const struct kernel_param *kp) {
  unsigned long addr;

  int ret;
  ret = kstrtoul(val, 0, &addr);
  if (ret) {
    pr_err("free_set: kstrtoul error\n");
    return -EINVAL;
  }

  void *ptr = (void *)addr;
  ret = allocator_free(ptr);
  if (ret != ALLOC_OK)
    pr_err("free_set: failed to free 0x%px (ret=%d)\n", ptr, ret);
  else
    pr_info("free_set: freed memory at 0x%px\n", ptr);

  return (ret == ALLOC_OK) ? 0 : -EINVAL;
}

static int stats_get(char *buf, const struct kernel_param *kp) {
  struct stats_info *s;
  s = allocator_get_stats();

  return scnprintf(buf, PAGE_SIZE,
                   "Total: %zu KB | Free: %zu KB | Allocated: %zu KB | "
                   "Fragmentation: %zu%%\n",
                   s->total_memory / 1024, s->free_memory / 1024,
                   s->allocated_memory / 1024, s->fragmentation_percent);
}

static int bitmap_info_get(char *buf, const struct kernel_param *kp) {
  return allocator_format_bitmap_info(buf, PAGE_SIZE);
}

static const struct kernel_param_ops param_ops_alloc = {
    .set = alloc_set,
    .get = NULL,
};

static const struct kernel_param_ops param_ops_free = {
    .set = free_set,
    .get = NULL,
};

static const struct kernel_param_ops param_ops_stats = {
    .set = NULL,
    .get = stats_get,
};

static const struct kernel_param_ops param_ops_bitmap_info = {
    .set = NULL,
    .get = bitmap_info_get,
};

module_param_cb(alloc, &param_ops_alloc, NULL, S_IWUSR);
MODULE_PARM_DESC(alloc, "Allocate memory");

module_param_cb(free, &param_ops_free, NULL, S_IWUSR);
MODULE_PARM_DESC(free, "Free memory");

module_param_cb(stats, &param_ops_stats, NULL, S_IRUSR);
MODULE_PARM_DESC(stats, "Print allocator statistics");

module_param_cb(bitmap_info, &param_ops_bitmap_info, NULL, S_IRUSR);
MODULE_PARM_DESC(bitmap_info, "Print bitmap state");
