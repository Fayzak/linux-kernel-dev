#include <linux/random.h>
#include <linux/slab.h>

#include "search.h"

#include "build.h"

int buildhashtable(struct bucket_search_ctx *ctx) {
  hash_init(ctx->htable);

  for (int i = 0; i < ctx->array_size; i++) {
    unsigned int val = get_random_u32() % ctx->array_size;

    struct hash_entry *entry = kmalloc(sizeof(struct hash_entry), GFP_KERNEL);
    if (!entry) {
      buildcleanuphashtable(ctx);
      return BS_NOMEM;
    }

    entry->value = val;

    hash_add(ctx->htable, &entry->node, val);
  }

  return 0;
}

void buildcleanuphashtable(struct bucket_search_ctx *ctx) {
  struct hash_entry *entry;
  struct hlist_node *tmp;
  int i;

  hash_for_each_safe(ctx->htable, i, tmp, entry, node) {
    hash_del(&entry->node);
    kfree(entry);
  }
}
