#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int idx;
static char ch_val;
static char my_str[64];

static int idx_set(const char *val, const struct kernel_param *kp) {
  int ret;
  ret = kstrtoint(val, 10, &idx);
  if (ret) {
    pr_err("idx_set: kstrtoint error\n");
    return ret;
  }

  if (idx < 0 || idx >= 64) {
    pr_err("idx_set: idx val not in range 0-63");
    return -EINVAL;
  }

  pr_info("idx_set value = %d\n", idx);
  return 0;
}

static int idx_get(char *val, const struct kernel_param *kp) {
  pr_info("idx_get: idx = %d\n", idx);
  return sprintf(val, "%d\n", idx);
}

static int ch_val_set(const char *val, const struct kernel_param *kp) {
  long ascii_code;
  int ret;

  ret = kstrtol(val, 0, &ascii_code);
  if (ret) {
    pr_err("ch_val_set: kstrtol error\n");
    return ret;
  }

  if (ascii_code < 0 || ascii_code > 127) {
    pr_err("ch_val_set: ch_val val not in range 0-127");
    return -EINVAL;
  }

  ch_val = (char)ascii_code;
  my_str[idx] = ch_val;
  pr_info("ch_val_set value = %c (ASCII %d), my_str[%d] = %c\n", ch_val, ch_val,
          idx, my_str[idx]);
  return 0;
}

static int ch_val_get(char *val, const struct kernel_param *kp) {
  pr_info("ch_val_get: ch_val = %c (ASCII %d)\n", ch_val, ch_val);
  return sprintf(val, "%c\n", ch_val);
}

static int my_str_get(char *val, const struct kernel_param *kp) {
  pr_info("my_str_get: my_str = %s\n", my_str);
  return sprintf(val, "%s\n", my_str);
}

static const struct kernel_param_ops idx_params = {
    .set = idx_set,
    .get = idx_get,
};
static const struct kernel_param_ops ch_val_params = {
    .set = ch_val_set,
    .get = ch_val_get,
};
static const struct kernel_param_ops my_str_params = {
    .set = NULL,
    .get = my_str_get,
};

module_param_cb(idx, &idx_params, &idx, 0664);
MODULE_PARM_DESC(idx, "Index of character in string");
module_param_cb(ch_val, &ch_val_params, &ch_val, 0664);
MODULE_PARM_DESC(ch_val, "ASCII-code of character");
module_param_cb(my_str, &my_str_params, &my_str, 0444);
MODULE_PARM_DESC(my_str, "The string that must contains Hello World!");

static int __init hello_world_init(void) {
  pr_info("init\n");
  return 0;
}

static void __exit hello_world_exit(void) { pr_info("exit\n"); }

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION("Hello World module with params");
MODULE_VERSION("1.0");