#include "stack.h"

static struct stack list_stack;

struct stack *stackgetstack(void) { return &list_stack; }