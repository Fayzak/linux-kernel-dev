#include <linux/bitmap.h>
#include <linux/bitops.h>

#include "bitmap.h"

void bitmap_set_bits(unsigned char *bitmap, size_t start, size_t nbits) {
  bitmap_set((unsigned long *)bitmap, start, nbits);
}

void bitmap_clear_bits(unsigned char *bitmap, size_t start, size_t nbits) {
  bitmap_clear((unsigned long *)bitmap, start, nbits);
}

int bitmap_test_bit(size_t idx, unsigned char *bitmap) {
  return test_bit(idx, (unsigned long *)bitmap);
}

int bitmap_find_first_fit(const unsigned char *bitmap, size_t total_bits,
                          size_t num) {
  return find_next_zero_bit((const unsigned long *)bitmap, total_bits, num);
}
