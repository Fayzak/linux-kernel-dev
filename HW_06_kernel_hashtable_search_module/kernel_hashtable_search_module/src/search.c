#include <linux/bsearch.h>
#include <linux/slab.h>
#include <linux/sort.h>
#include <linux/string.h>

#include "search.h"

static struct bucket_search_ctx ctx;

static int compare_entries(const void *a, const void *b) {
  const struct hash_entry *entry_a = *(const struct hash_entry **)a;
  const struct hash_entry *entry_b = *(const struct hash_entry **)b;

  if (entry_a->value < entry_b->value)
    return -1;
  if (entry_a->value > entry_b->value)
    return 1;
  return 0;
}

static int compare_value(const void *key, const void *element) {
  const unsigned int *search_value = (const unsigned int *)key;
  const struct hash_entry *entry = *(const struct hash_entry **)element;

  if (*search_value < entry->value)
    return -1;
  if (*search_value > entry->value)
    return 1;
  return 0;
}

int search(struct bucket_search_ctx *ctx, unsigned int x) {
  if (x >= ctx->array_size) {
    ctx->last_found = 0;
    ctx->last_value = 0;
    ctx->last_bucket = 0;
    return BS_NOT_FOUND;
  }

  unsigned int bucket = hash_min(x, HASH_BITS_COUNT);
  ctx->last_bucket = bucket;

  struct hash_entry *entry;
  int count = 0;
  hlist_for_each_entry(entry, &ctx->htable[bucket], node) { count++; }

  if (count == 0) {
    ctx->last_found = 0;
    ctx->last_value = 0;
    return BS_NOT_FOUND;
  }

  struct hash_entry **entries =
      kmalloc(count * sizeof(struct hash_entry *), GFP_KERNEL);
  if (!entries) {
    ctx->last_found = 0;
    ctx->last_value = 0;
    return BS_NOMEM;
  }

  int i = 0;
  hlist_for_each_entry(entry, &ctx->htable[bucket], node) {
    entries[i++] = entry;
  }

  sort(entries, count, sizeof(struct hash_entry *), compare_entries, NULL);

  struct hash_entry **found =
      bsearch(&x, entries, count, sizeof(struct hash_entry *), compare_value);

  if (found) {
    ctx->last_found = 1;
    ctx->last_value = (*found)->value;
  } else {
    ctx->last_found = 0;
    ctx->last_value = 0;
  }

  kfree(entries);

  return ctx->last_found ? BS_OK : BS_NOT_FOUND;
}

void searchresult(struct bucket_search_ctx *ctx, unsigned char *str) {
  snprintf(str, PAGE_SIZE, "found=%d value=%d bucket=%d\n", ctx->last_found,
           ctx->last_value, ctx->last_bucket);
}

int searchrebuild(struct bucket_search_ctx *ctx) {
  buildcleanuphashtable(ctx);
  return buildhashtable(ctx);
}

int searchsetbucketid(struct bucket_search_ctx *ctx, unsigned int id) {
  if (id >= (1U << HASH_BITS_COUNT))
    return BS_INVALID;

  ctx->current_bucket_id = id;
  return BS_OK;
}

int searchgetbucketdump(struct bucket_search_ctx *ctx, unsigned char *str) {

  unsigned int bucket = ctx->current_bucket_id;
  int len = 0;

  struct hash_entry *entry;
  int count = 0;
  hlist_for_each_entry(entry, &ctx->htable[bucket], node) { count++; }

  if (count == 0) {
    len = snprintf(str, PAGE_SIZE, "bucket=%u len=0:\n", bucket);
    return BS_OK;
  }

  struct hash_entry **entries =
      kmalloc(count * sizeof(struct hash_entry *), GFP_KERNEL);
  if (!entries) {
    return BS_NOMEM;
  }

  int i = 0;
  hlist_for_each_entry(entry, &ctx->htable[bucket], node) {
    entries[i++] = entry;
  }

  sort(entries, count, sizeof(struct hash_entry *), compare_entries, NULL);

  len = snprintf(str, PAGE_SIZE, "bucket=%u len=%d:", bucket, count);
  for (i = 0; i < count; i++) {
    len += snprintf(str + len, PAGE_SIZE - len, " %u", entries[i]->value);
  }
  len += snprintf(str + len, PAGE_SIZE - len, "\n");

  kfree(entries);
  return BS_OK;
}

struct bucket_search_ctx *searchgetctx(void) { return &ctx; }
