#include "token.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

Token *tokenizer(char *input)
{
    // Worst case: every character produces a token AND an implicit multiply.
    Token *res = (Token *)calloc(2 * strlen(input) + 1, sizeof(Token));
    int i = 0;
    char *ptr = input;
    while (*ptr)
    {
        // implicit multiplication: 2x, 2(x+1), (x+1)(x-1), x(x+1)
        // must run first, before isalpha/isdigit consume the current char
        if (i > 0) {
            TokenType prev = res[i-1].type;
            bool prev_can_multiply = (prev == TOKEN_NUMBER || prev == TOKEN_RPAREN);
            bool curr_starts_group = (isalpha(*ptr) || *ptr == '(');
            if (prev_can_multiply && curr_starts_group) {
                res[i].type   = TOKEN_OPERATOR;
                res[i].symbol = '*';
                i++;
            }

            // Reject adjacent variables like "xx" instead of treating them as implicit multiplication.
            if (res[i-1].type == TOKEN_VARIABLE && isalpha(*ptr)) {
                free(res);
                return NULL;
            }

            if (res[i-1].type == TOKEN_VARIABLE && *ptr == '(') {
                res[i].type   = TOKEN_OPERATOR;
                res[i].symbol = '*';
                i++;
            }
        }
        if (isalpha(*ptr))
        {
            res[i].type   = TOKEN_VARIABLE;
            res[i].symbol = *ptr;
            i++;
            ptr++;
            continue;
        }
        if (isdigit(*ptr) || *ptr == '.')
        {
            char *endptr;
            double val = strtod(ptr, &endptr);
            if (ptr != endptr)
            {
                res[i].type  = TOKEN_NUMBER;
                res[i].value = val;
                i++;
                ptr = endptr;
                continue;
            }
            else
            {
                free(res);
                return NULL;
            }
        }
        switch (*ptr)
        {
        case ' ':
        case '\t':
            break;
        case '-':
            // unary minus: emit 'n' (negation) operator instead of inserting '0 -'
            if (i == 0 || res[i-1].type == TOKEN_OPERATOR || res[i-1].type == TOKEN_LPAREN) {
                res[i].type   = TOKEN_OPERATOR;
                res[i].symbol = 'n';
            } else {
                res[i].type   = TOKEN_OPERATOR;
                res[i].symbol = '-';
            }
            i++;
            break;
        case '+':
        case '*':
        case '/':
        case '^':
            res[i].type   = TOKEN_OPERATOR;
            res[i].symbol = *ptr;
            i++;
            break;
        case '(':
            res[i].type   = TOKEN_LPAREN;
            res[i].symbol = *ptr;
            i++;
            break;
        case ')':
            res[i].type   = TOKEN_RPAREN;
            res[i].symbol = *ptr;
            i++;
            break;
        default:
            free(res);
            return NULL;
        }
        ptr++;
    }
    res[i].type = TOKEN_EOF;
    return res;
}
