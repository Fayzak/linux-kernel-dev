#include <linux/init.h>
#include <linux/module.h>

#include "search.h"

static unsigned int array_size = 1024;
module_param(array_size, uint, 0644);
MODULE_PARM_DESC(array_size,
                 "Number of array elements and upper bound of values");

static int __init kernel_hashtable_search_init(void) {

  if (array_size == 0) {
    pr_err("Error: array_size must be > 0\n");
    return -EINVAL;
  }

  struct bucket_search_ctx *ctx = searchgetctx();

  ctx->array_size = array_size;
  ctx->last_found = 0;
  ctx->last_value = 0;
  ctx->last_bucket = 0;
  ctx->current_bucket_id = 0;

  int ret = buildhashtable(ctx);
  if (ret) {
    pr_err("kernel_hashtable_search_module error building hashtable: %d\n",
           ret);
    return ret;
  }

  pr_info("kernel_hashtable_search_module loaded successfully: array_size=%u\n",
          array_size);
  return 0;
}

static void __exit kernel_hashtable_search_exit(void) {
  struct bucket_search_ctx *ctx = searchgetctx();
  buildcleanuphashtable(ctx);
  pr_info("kernel_hashtable_search_module unloaded\n");
}

module_init(kernel_hashtable_search_init);
module_exit(kernel_hashtable_search_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Timofey Zaika");
MODULE_DESCRIPTION(
    "Hastable Search Module based on hlist_node, bsearch and sort");
MODULE_VERSION("1.0");
