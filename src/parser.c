#include "parser.h"
#include "stack.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool is_supported_function_name(const char *name) {
    return strcmp(name, "sqrt") == 0 || strcmp(name, "log") == 0 ||
           strcmp(name, "sin") == 0 || strcmp(name, "cos") == 0;
}

int get_precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^' || op == 'n') return 3;  // 'n' = unary negation, same prec as ^
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
        } else if (t.type == TOKEN_FUNCTION) {
            push(stack, t);
        } else if (t.type == TOKEN_LPAREN) {
            push(stack, t);
        } else if (t.type == TOKEN_RPAREN) {
            while (!is_empty(stack) && peek(stack).type != TOKEN_LPAREN)
                postfix[out_idx++] = pop(stack);
            if (is_empty(stack)) {
                free(postfix);
                free_stack(stack);
                return NULL;
            }
            pop(stack); // discard left paren
            if (!is_empty(stack) && peek(stack).type == TOKEN_FUNCTION) {
                postfix[out_idx++] = pop(stack);
            }
        } else if (t.type == TOKEN_OPERATOR) {
            int  prec       = get_precedence(t.symbol);
            bool left_assoc = (t.symbol != '^' && t.symbol != 'n'); // ^ and n are right-associative
            while (!is_empty(stack) &&
                   (peek(stack).type == TOKEN_OPERATOR || peek(stack).type == TOKEN_FUNCTION)) {
                if (peek(stack).type == TOKEN_FUNCTION) {
                    postfix[out_idx++] = pop(stack);
                    continue;
                }
                int prec_top = get_precedence(peek(stack).symbol);
                if ((left_assoc && prec_top >= prec) || (!left_assoc && prec_top > prec))
                    postfix[out_idx++] = pop(stack);
                else
                    break;
            }
            push(stack, t);
        }
    }

    while (!is_empty(stack)) {
        if (peek(stack).type == TOKEN_LPAREN || peek(stack).type == TOKEN_RPAREN) {
            free(postfix);
            free_stack(stack);
            return NULL;
        }
        postfix[out_idx++] = pop(stack);
    }

    postfix[out_idx].type = TOKEN_EOF;
    free_stack(stack);
    return postfix;
}

static void print_postfix(Token *tokens) {
    for (int i = 0; tokens[i].type != TOKEN_EOF; i++) {
        if      (tokens[i].type == TOKEN_NUMBER)   printf("%g ",  tokens[i].value);
        else if (tokens[i].type == TOKEN_VARIABLE) printf("%c ",  tokens[i].symbol);
        else if (tokens[i].type == TOKEN_OPERATOR) printf("%c ",  tokens[i].symbol);
        else if (tokens[i].type == TOKEN_FUNCTION) printf("%s ",  tokens[i].funcName);
    }
    printf("\n");
}

static char *copy_with_closed_parentheses(const char *input) {
    size_t input_len = strlen(input);
    int open_parens = 0;

    for (size_t i = 0; i < input_len; i++) {
        if (input[i] == '(') {
            open_parens++;
        } else if (input[i] == ')' && open_parens > 0) {
            open_parens--;
        }
    }

    char *normalized = (char *)malloc(input_len + (size_t)open_parens + 1);
    if (normalized == NULL) return NULL;

    memcpy(normalized, input, input_len);
    for (int i = 0; i < open_parens; i++) {
        normalized[input_len + (size_t)i] = ')';
    }
    normalized[input_len + (size_t)open_parens] = '\0';
    return normalized;
}

static bool postfix_has_valid_shape(Token *postfix) {
    int stack_depth = 0;

    for (int i = 0; postfix[i].type != TOKEN_EOF; i++) {
        Token t = postfix[i];
        if (t.type == TOKEN_NUMBER || t.type == TOKEN_VARIABLE) {
            stack_depth++;
        } else if (t.type == TOKEN_FUNCTION || (t.type == TOKEN_OPERATOR && t.symbol == 'n')) {
            if (stack_depth < 1) return false;
        } else if (t.type == TOKEN_OPERATOR) {
            if (stack_depth < 2) return false;
            stack_depth--;
        } else {
            return false;
        }
    }

    return stack_depth == 1;
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
        } else if (t.type == TOKEN_FUNCTION) {
            if (top < 0 || !is_supported_function_name(t.funcName)) return NAN;
            switch (t.funcName[0]) {
                case 's':
                    if (strcmp(t.funcName, "sqrt") == 0) {
                        if (stack[top] < 0.0) return NAN;
                        stack[top] = sqrt(stack[top]);
                    } else if (strcmp(t.funcName, "sin") == 0) {
                        stack[top] = sin(stack[top]);
                    } else return NAN;
                    break;
                case 'l':
                    if (stack[top] <= 0.0) return NAN;
                    stack[top] = log(stack[top]);
                    break;
                case 'c':
                    stack[top] = cos(stack[top]);
                    break;
                default:
                    return NAN;
            }
        } else if (t.type == TOKEN_OPERATOR) {
            if (t.symbol == 'n') {
                // Unary negation: one operand
                if (top < 0) return NAN;
                stack[top] = -stack[top];
            } else {
                if (top < 1) return NAN; // not enough operands (e.g. trailing operator)
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
    }
    if (top != 0) return NAN; // leftover values = malformed expression
    return stack[top];
}

Token *parse(char *input) {
    char *normalized_input = copy_with_closed_parentheses(input);
    if (normalized_input == NULL) {
        return NULL;
    }

    Token *infix = tokenizer(normalized_input);
    if (infix == NULL) {
        printf("Error: tokenizer failed for: %s\n", input);
        free(normalized_input);
        return NULL;
    }
    Token *postfix = infix_to_postfix(infix);
    if (postfix == NULL) {
        printf("Error: parser failed for: %s\n", input);
        free(infix);
        free(normalized_input);
        return NULL;
    }
    if (!postfix_has_valid_shape(postfix)) {
        printf("Error: malformed expression: %s\n", input);
        free(postfix);
        free(infix);
        free(normalized_input);
        return NULL;
    }
    printf("Postfix: ");
    print_postfix(postfix);
    free(infix);
    free(normalized_input);
    return postfix;
}
