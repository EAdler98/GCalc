#ifndef STACK_H
#define STACK_H

#include "token.h"
#include <stdbool.h>

typedef struct {
    Token *data;
    int top;
    int capacity;
} TokenStack;

TokenStack *create_stack(int capacity);
void        free_stack(TokenStack *stack);
bool        is_empty(TokenStack *stack);
void        push(TokenStack *stack, Token t);
Token       pop(TokenStack *stack);
Token       peek(TokenStack *stack);

#endif
