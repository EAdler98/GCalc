#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

TokenStack *create_stack(int capacity) {
    TokenStack *stack = (TokenStack *)malloc(sizeof(TokenStack));
    stack->data     = (Token *)malloc(capacity * sizeof(Token));
    stack->capacity = capacity;
    stack->top      = -1;
    return stack;
}

void free_stack(TokenStack *stack) {
    if (stack) {
        free(stack->data);
        free(stack);
    }
}

bool is_empty(TokenStack *stack) {
    return stack->top == -1;
}

void push(TokenStack *stack, Token t) {
    if (stack->top == stack->capacity - 1) {
        printf("Error: Stack Overflow!\n");
        return;
    }
    stack->data[++stack->top] = t;
}

Token pop(TokenStack *stack) {
    if (is_empty(stack)) {
        printf("Error: Stack Underflow!\n");
        Token empty = {TOKEN_EOF, 0, 0};
        return empty;
    }
    return stack->data[stack->top--];
}

Token peek(TokenStack *stack) {
    if (is_empty(stack)) {
        Token empty = {TOKEN_EOF, 0, 0};
        return empty;
    }
    return stack->data[stack->top];
}
