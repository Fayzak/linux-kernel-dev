#ifndef BUILD_H
#define BUILD_H

#include "linux/types.h"
#include <linux/hashtable.h>

#define BS_OK 0              /* операция успешна */
#define BS_INVALID -EINVAL   /* неверный параметр */
#define BS_NOMEM -ENOMEM     /* недостаточно памяти */
#define BS_NOT_FOUND -ENOENT /* число не найдено */
#define HASH_BITS_COUNT 6    /* размер хэш-таблицы */

struct hash_entry {
  struct hlist_node node; /* узел для встраивания в хэш-таблицу */
  unsigned int value;     /* значение элемента */
};

struct bucket_search_ctx {
  unsigned int array_size;

  DECLARE_HASHTABLE(htable, HASH_BITS_COUNT); /* хэш-таблица */

  /* Для хранения результата последнего поиска */
  int last_found;
  unsigned int last_value;
  unsigned int last_bucket;

  /* Для bucket_dump */
  unsigned int current_bucket_id;
};

int buildhashtable(struct bucket_search_ctx *ctx);
void buildcleanuphashtable(struct bucket_search_ctx *ctx);

#endif