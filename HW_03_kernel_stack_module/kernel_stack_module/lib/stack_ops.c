#include <linux/slab.h>

#include "stack.h"

#include "stack_ops.h"

void stack_init(void) {
  struct stack *list_stack = stackgetstack();
  INIT_LIST_HEAD(&list_stack->elements);
  list_stack->size = 0;
}

int stack_push(int value) {
  struct stack_entry *entry = kmalloc(sizeof(struct stack_entry), GFP_KERNEL);
  if (!entry) {
    return STACK_NOMEM;
  }

  entry->data = value;

  struct stack *list_stack = stackgetstack();
  list_add(&entry->list, &list_stack->elements);
  list_stack->size++;

  return STACK_OK;
}

int stack_pop(void) {
  if (stack_is_empty()) {
    return STACK_EMPTY;
  }

  struct stack *list_stack = stackgetstack();
  struct stack_entry *top =
      list_first_entry(&list_stack->elements, struct stack_entry, list);
  int value = top->data;

  list_del(&top->list);
  kfree(top);
  list_stack->size--;

  return value;
}

int stack_peek(void) {
  if (stack_is_empty()) {
    return STACK_EMPTY;
  }

  struct stack *list_stack = stackgetstack();
  struct stack_entry *top =
      list_first_entry(&list_stack->elements, struct stack_entry, list);

  return top->data;
}

int stack_is_empty(void) {
  struct stack *list_stack = stackgetstack();
  return list_empty(&list_stack->elements);
}

int stack_size(void) {
  struct stack *list_stack = stackgetstack();
  return list_stack->size;
}

void stack_clear(void) {
  while (!stack_is_empty()) {
    stack_pop();
  }
}