#include <linux/bitmap.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/vmalloc.h>

#include "allocator.h"
#include "bitmap.h"

static struct memory_allocator g_allocator;
static struct allocation_info g_allocations[ALLOC_TOTAL_BLOCKS];
static size_t g_alloc_count;

int allocator_init(void) {
  g_allocator.memory_pool = vmalloc(ALLOC_POOL_SIZE);
  if (!g_allocator.memory_pool) {
    return ALLOC_NOMEM;
  }

  g_allocator.bitmap =
      (unsigned char *)bitmap_zalloc(ALLOC_BITMAP_SIZE * 8, GFP_KERNEL);
  if (!g_allocator.bitmap) {
    vfree(g_allocator.memory_pool);
    g_allocator.memory_pool = NULL;
    return ALLOC_NOMEM;
  }

  g_allocator.total_blocks = ALLOC_TOTAL_BLOCKS;
  g_allocator.block_size = ALLOC_BLOCK_SIZE;
  spin_lock_init(&g_allocator.lock);

  memset(g_allocations, 0, sizeof(g_allocations));
  g_alloc_count = 0;

  return ALLOC_OK;
}

void *allocator_alloc(size_t bytes) {
  if (bytes == 0) {
    return NULL;
  }

  size_t num_blocks = (bytes + ALLOC_BLOCK_SIZE - 1) / ALLOC_BLOCK_SIZE;
  if (num_blocks > ALLOC_TOTAL_BLOCKS) {
    return NULL;
  }

  unsigned long flags;
  spin_lock_irqsave(&g_allocator.lock, flags);

  int start = bitmap_find_first_fit(g_allocator.bitmap,
                                    g_allocator.total_blocks, num_blocks);
  if (start < 0) {
    spin_unlock_irqrestore(&g_allocator.lock, flags);
    return NULL;
  }

  bitmap_set_bits(g_allocator.bitmap, start, num_blocks);

  void *ptr = (void *)((unsigned char *)g_allocator.memory_pool +
                       start * g_allocator.block_size);

  if (g_alloc_count < ALLOC_TOTAL_BLOCKS) {
    g_allocations[g_alloc_count].start_block = start;
    g_allocations[g_alloc_count].num_blocks = num_blocks;
    g_alloc_count++;
  }

  spin_unlock_irqrestore(&g_allocator.lock, flags);

  return ptr;
}

int allocator_free(void *ptr) {
  unsigned long flags;

  if (!ptr)
    return ALLOC_INVALID;

  if ((unsigned long)ptr < (unsigned long)g_allocator.memory_pool ||
      (unsigned long)ptr >=
          (unsigned long)g_allocator.memory_pool + ALLOC_POOL_SIZE)
    return ALLOC_INVALID;

  size_t offset = (unsigned long)ptr - (unsigned long)g_allocator.memory_pool;
  if (offset % ALLOC_BLOCK_SIZE != 0)
    return ALLOC_INVALID;

  size_t idx = offset / ALLOC_BLOCK_SIZE;

  spin_lock_irqsave(&g_allocator.lock, flags);

  int start_block = -1;
  size_t num_blocks = 0;
  int found = 0;
  for (size_t i = 0; i < g_alloc_count; i++) {
    if (g_allocations[i].start_block == idx) {
      start_block = (int)g_allocations[i].start_block;
      num_blocks = g_allocations[i].num_blocks;
      found = 1;

      g_alloc_count--;
      if (i < g_alloc_count)
        memmove(&g_allocations[i], &g_allocations[i + 1],
                (g_alloc_count - i) * sizeof(struct allocation_info));
      break;
    }
  }

  if (!found) {
    spin_unlock_irqrestore(&g_allocator.lock, flags);
    return ALLOC_NOT_FOUND;
  }

  bitmap_clear_bits(g_allocator.bitmap, start_block, num_blocks);

  spin_unlock_irqrestore(&g_allocator.lock, flags);

  return ALLOC_OK;
}

struct stats_info *allocator_get_stats(void) {
  static struct stats_info stats;
  unsigned long flags;

  memset(&stats, 0, sizeof(stats));

  spin_lock_irqsave(&g_allocator.lock, flags);

  stats.total_blocks = g_allocator.total_blocks;
  stats.total_memory = g_allocator.total_blocks * g_allocator.block_size;

  size_t free_blocks = 0;
  size_t max_free_blocks = 0;
  size_t current_free_blocks = 0;
  for (size_t i = 0; i < g_allocator.total_blocks; i++) {
    if (!bitmap_test_bit(i, g_allocator.bitmap)) {
      free_blocks++;
      current_free_blocks++;
      if (current_free_blocks > max_free_blocks) {
        max_free_blocks = current_free_blocks;
      }
    } else {
      current_free_blocks = 0;
    }
  }

  stats.free_blocks = free_blocks;
  stats.allocated_blocks = stats.total_blocks - free_blocks;

  stats.free_memory = free_blocks * g_allocator.block_size;
  stats.allocated_memory = stats.allocated_blocks * g_allocator.block_size;

  if (stats.free_memory == 0) {
    stats.fragmentation_percent = 0;
  } else {
    size_t largest_free_memory = max_free_blocks * g_allocator.block_size;
    stats.fragmentation_percent =
        100 * (stats.free_memory - largest_free_memory) / stats.free_memory;
  }

  spin_unlock_irqrestore(&g_allocator.lock, flags);

  return &stats;
}

int allocator_format_bitmap_info(char *buf, size_t buf_size) {
  size_t i;
  int written = 0;
  size_t allocated_blocks = 0;

  written += scnprintf(buf + written, buf_size - written,
                       "Bitmap (%zu blocks): [", g_allocator.total_blocks);

  for (i = 0; i < ALLOC_TOTAL_BLOCKS; i++) {
    if (i > 0 && i % 10 == 0) {
      written += scnprintf(buf + written, buf_size - written, " ");
      if (written >= (int)buf_size - 3)
        break;
    }
    if (bitmap_test_bit(i, g_allocator.bitmap)) {
      written += scnprintf(buf + written, buf_size - written, "X");
      allocated_blocks++;
    } else {
      written += scnprintf(buf + written, buf_size - written, ".");
    }
  }

  written += scnprintf(buf + written, buf_size - written,
                       "]\nAllocated blocks: %zu / %zu\n", allocated_blocks,
                       g_allocator.total_blocks);

  written += scnprintf(buf + written, buf_size - written, "Allocations:\n");
  for (i = 0; i < g_alloc_count; i++) {
    written += scnprintf(buf + written, buf_size - written,
                         "  #%zu: blocks %zu-%zu (%zu blocks)\n", i,
                         g_allocations[i].start_block,
                         g_allocations[i].start_block +
                             g_allocations[i].num_blocks - 1,
                         g_allocations[i].num_blocks);
  }

  return written;
}

void allocator_cleanup(void) {
  unsigned long flags;

  spin_lock_irqsave(&g_allocator.lock, flags);

  if (g_allocator.bitmap) {
    bitmap_free((const unsigned long *)g_allocator.bitmap);
    g_allocator.bitmap = NULL;
  }

  if (g_allocator.memory_pool) {
    vfree(g_allocator.memory_pool);
    g_allocator.memory_pool = NULL;
  }

  g_alloc_count = 0;
  memset(g_allocations, 0, sizeof(g_allocations));

  spin_unlock_irqrestore(&g_allocator.lock, flags);
}
