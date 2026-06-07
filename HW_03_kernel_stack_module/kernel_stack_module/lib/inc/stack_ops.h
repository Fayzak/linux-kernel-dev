#ifndef STACKOPSH
#define STACKOPSH

void stack_init(void);
int stack_push(int value);
int stack_pop(void);
int stack_peek(void);
int stack_is_empty(void);
int stack_size(void);
void stack_clear(void);

#endif