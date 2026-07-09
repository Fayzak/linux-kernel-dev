#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "sync.h"

static int run_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;
  int ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("run_set: kstrtouint error\n");
    return -EINVAL;
  }

  if (x == 0)
    return 0;

  struct sync_ctx *ctx = syncgetctx();
  int run_ret = syncrun(ctx);

  if (run_ret < 0)
    return run_ret;

  return 0;
}

static int result_get(char *buf, const struct kernel_param *kp) {
  struct sync_ctx *ctx = syncgetctx();
  syncresult(ctx, buf);
  return strlen(buf);
}

// --- LOCK_TYPE ---
static int lock_type_set(const char *val, const struct kernel_param *kp) {
  unsigned int trigger;
  int ret = kstrtouint(val, 10, &trigger);
  if (ret) {
    pr_err("lock_type_set: kstrtouint error\n");
    return SD_INVALID;
  }

  if (trigger > 2) {
    pr_err("lock_type_set: lock_type must be 0, 1 or 2\n");
    return SD_INVALID;
  }

  struct sync_ctx *ctx = syncgetctx();
  if (ctx->last_run_result > 0) {
    pr_err("lock_type_set: test is currently running\n");
    return SD_INVALID;
  }
  ctx->lock_type = trigger;
  return SD_OK;
}

static int stats_get(char *buf, const struct kernel_param *kp) {
  struct sync_ctx *ctx = syncgetctx();
  syncstats(ctx, buf);
  return strlen(buf);
}

static int reset_set(const char *val, const struct kernel_param *kp) {
  unsigned int trigger;
  int ret = kstrtouint(val, 10, &trigger);
  if (ret) {
    pr_err("reset_set: kstrtouint error\n");
    return SD_INVALID;
  }

  if (trigger == 0)
    return SD_OK;

  struct sync_ctx *ctx = syncgetctx();
  syncreset(ctx);
  return SD_OK;
}

static const struct kernel_param_ops param_ops_run = {
    .set = run_set,
};
module_param_cb(run, &param_ops_run, NULL, S_IWUSR);
MODULE_PARM_DESC(run, "Run the test with the current parameters (write 1)");

static const struct kernel_param_ops param_ops_result = {
    .get = result_get,
};
module_param_cb(result, &param_ops_result, NULL, S_IRUGO);
MODULE_PARM_DESC(result, "Get the result of the last run");

static const struct kernel_param_ops param_ops_lock_type = {
    .set = lock_type_set,
};
module_param_cb(lock_type, &param_ops_lock_type, NULL, S_IWUSR);
MODULE_PARM_DESC(lock_type, "Change lock type (0/1/2)");

static const struct kernel_param_ops param_ops_stats = {
    .get = stats_get,
};
module_param_cb(stats, &param_ops_stats, NULL, S_IRUGO);
MODULE_PARM_DESC(stats, "Statistics on expectations for the last launch");

static const struct kernel_param_ops param_ops_reset = {
    .set = reset_set,
};
module_param_cb(reset, &param_ops_reset, NULL, S_IWUSR);
MODULE_PARM_DESC(reset,
                 "Reset counter and statistics (write any non-zero value)");
