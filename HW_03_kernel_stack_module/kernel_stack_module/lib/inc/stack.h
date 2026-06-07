#ifndef STACKH
#define STACKH

#include <linux/list.h>

#define STACK_OK 0       /* Операция успешна */
#define STACK_EMPTY -1   /* Стек пуст */
#define STACK_NOMEM -2   /* Нет памяти */
#define STACK_INVALID -3 /* Неверный параметр */

struct stack {
  struct list_head elements; /* Голова связного списка */
  int size;                  /* Текущее количество элементов */
};

struct stack_entry {
  struct list_head list; /* Узел связного списка */
  int data;              /* Данные (целое число) */
};

struct stack *stackgetstack(void);

#endif