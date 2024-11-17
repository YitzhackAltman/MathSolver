#include <stdlib.h>

typedef struct {
  char* data;
  size_t size;
  size_t capacity;
}Stack;


Stack* stack_create(void) {
  Stack* stack = malloc(sizeof(Stack));
  if (stack == NULL) {
    return NULL;
  }
  
  int default_cap = 10;
  stack->capacity = default_cap;
  stack->size = 0; 
  stack->data = malloc(stack->capacity * sizeof(char));

  return stack;
}

void stack_delete(Stack** stack) {
  free((*stack)->data);
  free(*stack);
  *stack = 0;
}

void stack_resize(Stack* stack) {
  int default_multiplayer = 2;
  stack->capacity *= default_multiplayer;
  char* data = malloc(stack->capacity * sizeof(char));
  if (data == NULL) {
    fprintf(stderr, "[ERROR]: Could not allocate enought memory\n");
    return;
  }

  for(size_t i =0; i < stack->size; ++i) {
    data[i] = stack->data[i];
  }

  free(stack->data);
  stack->data = data;
  
}

void stack_push(Stack* stack, char data) {
  stack->data[stack->size++] = data; 
}

int stack_is_empty(Stack* stack) {
  return stack->size == 0;
}

char stack_pop(Stack* stack) {
  assert(stack->size > 0);
  return stack->data[--stack->size];
}
