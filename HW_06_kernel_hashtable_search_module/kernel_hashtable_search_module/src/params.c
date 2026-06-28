#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "build.h"
#include "search.h"

static int search_set(const char *val, const struct kernel_param *kp) {
  unsigned int x;

  int ret;
  ret = kstrtouint(val, 10, &x);
  if (ret) {
    pr_err("search_set: kstrtouint error\n");
    return -EINVAL;
  }

  struct bucket_search_ctx *ctx = searchgetctx();
  int res = search(ctx, x);
  if (res == BS_OK)
    pr_info("search_set: search ok\n");
  else if (res == BS_NOT_FOUND)
    pr_info("search_set: search not found\n");
  else
    pr_info("search_set: search no mem\n");

  return 0;
}

static int result_get(char *buf, const struct kernel_param *kp) {
  unsigned char result[64];

  struct bucket_search_ctx *ctx = searchgetctx();
  searchresult(ctx, result);

  return snprintf(buf, sizeof(result), "%s\n", result);
}

static int rebuild_set(const char *val, const struct kernel_param *kp) {
  int trigger;

  int ret;
  ret = kstrtoint(val, 10, &trigger);
  if (ret) {
    pr_err("rebuild_set: kstrtoint error\n");
    return -EINVAL;
  }

  if (trigger == 0)
    return 0;

  struct bucket_search_ctx *ctx = searchgetctx();
  int res = searchrebuild(ctx);
  if (res == BS_OK)
    pr_info("rebuild_set: rebuild ok\n");
  else
    pr_info("rebuild_set: rebuild no mem\n");

  return 0;
}

static int bucket_id_set(const char *val, const struct kernel_param *kp) {
  unsigned int id;

  int ret;
  ret = kstrtouint(val, 10, &id);
  if (ret) {
    pr_err("bucket_id_set: kstrtouint error\n");
    return -EINVAL;
  }

  struct bucket_search_ctx *ctx = searchgetctx();
  int res = searchsetbucketid(ctx, id);
  if (res == BS_OK)
    pr_info("bucket_id_set: bucket_id ok\n");
  else
    pr_info("bucket_id_set: bucket_id invalid param\n");

  return 0;
}

static int bucket_dump_get(char *buf, const struct kernel_param *kp) {
  unsigned char result[128];

  struct bucket_search_ctx *ctx = searchgetctx();
  int res = searchgetbucketdump(ctx, result);
  if (res == BS_NOMEM) {
    pr_info("bucket_dump_get: bucket_dump no mem\n");
    return snprintf(buf, PAGE_SIZE, "error: no memory\n");
  }

  return snprintf(buf, sizeof(result), "%s\n", result);
}

static const struct kernel_param_ops param_ops_search = {
    .set = search_set,
    .get = NULL,
};

static const struct kernel_param_ops param_ops_result = {
    .set = NULL,
    .get = result_get,
};

static const struct kernel_param_ops param_ops_rebuild = {
    .set = rebuild_set,
    .get = NULL,
};

static const struct kernel_param_ops param_ops_bucket_id = {
    .set = bucket_id_set,
    .get = NULL,
};

static const struct kernel_param_ops param_ops_bucket_dump = {
    .set = NULL,
    .get = bucket_dump_get,
};

module_param_cb(search, &param_ops_search, NULL, S_IWUSR);
MODULE_PARM_DESC(search, "Start searching for a number in the hash table");

module_param_cb(result, &param_ops_result, NULL, S_IRUSR);
MODULE_PARM_DESC(result, "Get the result of the last search");

module_param_cb(rebuild, &param_ops_rebuild, NULL, S_IWUSR);
MODULE_PARM_DESC(rebuild, "Rebuild the hash table");

module_param_cb(bucket_id, &param_ops_bucket_id, NULL, S_IWUSR);
MODULE_PARM_DESC(bucket_id, "Set the bucket index to view the content");

module_param_cb(bucket_dump, &param_ops_bucket_dump, NULL, S_IRUSR);
MODULE_PARM_DESC(bucket_dump,
                 "Print the content of the bucket with the index bucket_id");
