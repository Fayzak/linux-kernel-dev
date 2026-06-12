#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "fifo_ops.h"

static int enqueue_set(const char *val, const struct kernel_param *kp) {
  int value;
  if (kstrtoint(val, 10, &value)) {
    pr_err("enqueue_set: kstrtoint error\n");
    return -EINVAL;
  }

  int ret;
  ret = fifo_enqueue(value);
  if (ret < 0) {
    pr_err("enqueue_set: fifo is full\n");
    return ret;
  }

  return 0;
}

static int dequeue_get(char *val, const struct kernel_param *kp) {
  int value = fifo_dequeue();
  if (value < 0) {
    pr_err("dequeue_get: fifo is empty\n");
    return snprintf(val, PAGE_SIZE, "error: %d\n", value);
  }
  return snprintf(val, PAGE_SIZE, "%d\n", value);
}

static int peek_get(char *val, const struct kernel_param *kp) {
  int value = fifo_peek();
  if (value < 0) {
    pr_err("peek_get: fifo is empty\n");
    return snprintf(val, PAGE_SIZE, "error: %d\n", value);
  }
  return snprintf(val, PAGE_SIZE, "%d\n", value);
}

static int size_get(char *val, const struct kernel_param *kp) {
  int value = fifo_size();
  return snprintf(val, PAGE_SIZE, "%d\n", value);
}

static int available_get(char *val, const struct kernel_param *kp) {
  int value = fifo_available();
  return snprintf(val, PAGE_SIZE, "%d\n", value);
}

static int is_empty_get(char *val, const struct kernel_param *kp) {
  int value = fifo_is_empty();
  return snprintf(val, PAGE_SIZE, "%d\n", value);
}

static int is_full_get(char *val, const struct kernel_param *kp) {
  int value = fifo_is_full();
  return snprintf(val, PAGE_SIZE, "%d\n", value);
}

static int clear_set(const char *val, const struct kernel_param *kp) {
  int value;
  if (kstrtoint(val, 10, &value)) {
    pr_err("clear_set: kstrtoint error\n");
    return -EINVAL;
  }

  fifo_clear();

  return 0;
}

static const struct kernel_param_ops param_ops_enqueue = {
    .set = enqueue_set,
    .get = NULL,
};
static const struct kernel_param_ops param_ops_dequeue = {
    .set = NULL,
    .get = dequeue_get,
};
static const struct kernel_param_ops param_ops_peek = {
    .set = NULL,
    .get = peek_get,
};
static const struct kernel_param_ops param_ops_size = {
    .set = NULL,
    .get = size_get,
};
static const struct kernel_param_ops param_ops_available = {
    .set = NULL,
    .get = available_get,
};
static const struct kernel_param_ops param_ops_is_empty = {
    .set = NULL,
    .get = is_empty_get,
};
static const struct kernel_param_ops param_ops_is_full = {
    .set = NULL,
    .get = is_full_get,
};
static const struct kernel_param_ops param_ops_clear = {
    .set = clear_set,
    .get = NULL,
};

module_param_cb(enqueue, &param_ops_enqueue, NULL, S_IWUSR);
MODULE_PARM_DESC(enqueue, "fifo enqueue");
module_param_cb(dequeue, &param_ops_dequeue, NULL, S_IRUSR);
MODULE_PARM_DESC(dequeue, "fifo dequeue");
module_param_cb(peek, &param_ops_peek, NULL, S_IRUSR);
MODULE_PARM_DESC(peek, "fifo peek");
module_param_cb(size, &param_ops_size, NULL, S_IRUSR);
MODULE_PARM_DESC(size, "fifo size");
module_param_cb(available, &param_ops_available, NULL, S_IRUSR);
MODULE_PARM_DESC(available, "fifo available");
module_param_cb(is_empty, &param_ops_is_empty, NULL, S_IRUSR);
MODULE_PARM_DESC(is_empty, "fifo is_empty");
module_param_cb(is_full, &param_ops_is_full, NULL, S_IRUSR);
MODULE_PARM_DESC(is_full, "fifo is_full");
module_param_cb(clear, &param_ops_clear, NULL, S_IWUSR);
MODULE_PARM_DESC(clear, "fifo clear");