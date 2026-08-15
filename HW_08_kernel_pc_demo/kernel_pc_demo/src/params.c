#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "producer.h"

static int run_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;
  int ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("run_set: kstrtouint error\n");
    return PC_INVALID;
  }

  struct pc_ctx *ctx = producergetctx();
  int run_ret = producerrun(ctx);

  return run_ret;
}

static int result_get(char *buf, const struct kernel_param *kp) {
  struct pc_ctx *ctx = producergetctx();
  producerresult(ctx, buf);
  return strlen(buf);
}

static int stats_get(char *buf, const struct kernel_param *kp) {
  struct pc_ctx *ctx = producergetctx();
  producerstats(ctx, buf);
  return strlen(buf);
}

static int consumer_type_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;
  int ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("consumer_type_set: kstrtouint error\n");
    return PC_INVALID;
  }

  struct pc_ctx *ctx = producergetctx();
  int consumer_type_ret = producerconsumertype(ctx, x);

  return consumer_type_ret;
}

static int reset_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;
  int ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("reset_set: kstrtouint error\n");
    return PC_INVALID;
  }

  struct pc_ctx *ctx = producergetctx();
  int reset_ret = producerreset(ctx);

  return reset_ret;
}

static const struct kernel_param_ops param_ops_run = {
    .set = run_set,
};
module_param_cb(run, &param_ops_run, NULL, S_IWUSR);
MODULE_PARM_DESC(run, "Run the test");

static const struct kernel_param_ops param_ops_result = {
    .get = result_get,
};
module_param_cb(result, &param_ops_result, NULL, S_IRUGO);
MODULE_PARM_DESC(result, "Get the result of the last run");

static const struct kernel_param_ops param_ops_stats = {
    .get = stats_get,
};
module_param_cb(stats, &param_ops_stats, NULL, S_IRUGO);
MODULE_PARM_DESC(stats, "Stats of produced/consumed/dropped");

static const struct kernel_param_ops param_ops_consumer_type = {
    .set = consumer_type_set,
};
module_param_cb(consumer_type, &param_ops_consumer_type, NULL, S_IWUSR);
MODULE_PARM_DESC(consumer_type, "Change consumer type (0/1)");

static const struct kernel_param_ops param_ops_reset = {
    .set = reset_set,
};
module_param_cb(reset, &param_ops_reset, NULL, S_IWUSR);
MODULE_PARM_DESC(reset, "Reset queue and stats");
