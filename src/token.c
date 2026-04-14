#include "token.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

Token *tokenizer(char *input)
{
    Token *res = (Token *)calloc(strlen(input), sizeof(Token) + 1);
    int i = 0;
    char *ptr = input;
    while (*ptr)
    {
        // implicit multiplication: 2x, 2(x+1), (a)(b)
        // must run first, before isalpha/isdigit consume the current char
        if (i > 0) {
            TokenType prev = res[i-1].type;
            bool prev_can_multiply = (prev == TOKEN_NUMBER || prev == TOKEN_VARIABLE || prev == TOKEN_RPAREN);
            bool curr_starts_group = (isalpha(*ptr) || *ptr == '(');
            if (prev_can_multiply && curr_starts_group) {
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
        case '+':
        case '-':
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
