#include "parser.h"
#include "stack.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

int get_precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^')              return 3;
    return 0;
}

Token *infix_to_postfix(Token *infix) {
    int len = 0;
    while (infix[len].type != TOKEN_EOF) len++;

    Token      *postfix = (Token *)malloc((len + 1) * sizeof(Token));
    TokenStack *stack   = create_stack(len);
    int out_idx = 0;

    for (int i = 0; i < len; i++) {
        Token t = infix[i];

        if (t.type == TOKEN_NUMBER || t.type == TOKEN_VARIABLE) {
            postfix[out_idx++] = t;
        } else if (t.type == TOKEN_LPAREN) {
            push(stack, t);
        } else if (t.type == TOKEN_RPAREN) {
            while (!is_empty(stack) && peek(stack).type != TOKEN_LPAREN)
                postfix[out_idx++] = pop(stack);
            pop(stack); // discard left paren
        } else if (t.type == TOKEN_OPERATOR) {
            int  prec       = get_precedence(t.symbol);
            bool left_assoc = (t.symbol != '^'); // ^ is right-associative
            while (!is_empty(stack) && peek(stack).type == TOKEN_OPERATOR) {
                int prec_top = get_precedence(peek(stack).symbol);
                if ((left_assoc && prec_top >= prec) || (!left_assoc && prec_top > prec))
                    postfix[out_idx++] = pop(stack);
                else
                    break;
            }
            push(stack, t);
        }
    }

    while (!is_empty(stack))
        postfix[out_idx++] = pop(stack);

    postfix[out_idx].type = TOKEN_EOF;
    free_stack(stack);
    return postfix;
}

static void print_postfix(Token *tokens) {
    for (int i = 0; tokens[i].type != TOKEN_EOF; i++) {
        if      (tokens[i].type == TOKEN_NUMBER)   printf("%g ",  tokens[i].value);
        else if (tokens[i].type == TOKEN_VARIABLE) printf("%c ",  tokens[i].symbol);
        else if (tokens[i].type == TOKEN_OPERATOR) printf("%c ",  tokens[i].symbol);
    }
    printf("\n");
}

double evaluate_postfix(Token *postfix, double x_value) {
    double stack[256];
    int top = -1;

    for (int i = 0; postfix[i].type != TOKEN_EOF; i++) {
        Token t = postfix[i];
        if (t.type == TOKEN_NUMBER) {
            stack[++top] = t.value;
        } else if (t.type == TOKEN_VARIABLE) {
            stack[++top] = x_value;
        } else if (t.type == TOKEN_OPERATOR) {
            double right = stack[top--];
            double left  = stack[top--];
            switch (t.symbol) {
                case '+': stack[++top] = left + right;        break;
                case '-': stack[++top] = left - right;        break;
                case '*': stack[++top] = left * right;        break;
                case '/':
                    if (right == 0.0) return NAN;
                    stack[++top] = left / right;
                    break;
                case '^': stack[++top] = pow(left, right);    break;
            }
        }
    }
    return stack[top];
}

Token *parse(char *input) {
    Token *infix = tokenizer(input);
    if (infix == NULL) {
        printf("Error: tokenizer failed for: %s\n", input);
        return NULL;
    }
    Token *postfix = infix_to_postfix(infix);
    printf("Postfix: ");
    print_postfix(postfix);
    free(infix);
    return postfix;
}
