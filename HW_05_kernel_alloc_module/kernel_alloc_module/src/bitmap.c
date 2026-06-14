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
  if (num == 0 || num > total_bits)
    return -1;

  size_t start = 0;

  while (start <= total_bits - num) {
    size_t bit_pos =
        find_next_zero_bit((const unsigned long *)bitmap, total_bits, start);

    if (bit_pos >= total_bits || bit_pos > total_bits - num)
      return -1;

    size_t next_occupied =
        find_next_bit((const unsigned long *)bitmap, total_bits, bit_pos + 1);

    if (next_occupied - bit_pos >= num) {
      return (int)bit_pos;
    }

    start = next_occupied;
  }

  return -1;
}
