#ifndef BITMAP_H
#define BITMAP_H

#include <linux/types.h>

void bitmap_set_bits(unsigned char *bitmap, size_t start, size_t nbits);
void bitmap_clear_bits(unsigned char *bitmap, size_t start, size_t nbits);
int bitmap_test_bit(size_t idx, unsigned char *bitmap);
int bitmap_find_first_fit(const unsigned char *bitmap, size_t total_bits,
                          size_t num);

#endif
