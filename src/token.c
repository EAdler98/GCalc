#include "token.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static int identifier_length(const char *text)
{
    int len = 0;
    while (isalpha((unsigned char)text[len])) len++;
    return len;
}

static bool is_variable_char(char c)
{
    return c == 'x';
}

static bool is_supported_function(const char *text, int len)
{
    return ((len == 3) && (strncmp(text, "log", 3) == 0 || strncmp(text, "sin", 3) == 0 || strncmp(text, "cos", 3) == 0)) ||
           ((len == 4) && (strncmp(text, "sqrt", 4) == 0));
}

static bool starts_valid_symbol(const char *text)
{
    int len = identifier_length(text);
    if (len == 1 && is_variable_char(text[0])) return true;
    return is_supported_function(text, len);
}

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
            bool prev_can_multiply = (prev == TOKEN_NUMBER || prev == TOKEN_RPAREN || prev == TOKEN_VARIABLE);
            bool curr_starts_group = (*ptr == '(') || (isalpha((unsigned char)*ptr) && starts_valid_symbol(ptr));
            if (prev_can_multiply && curr_starts_group) {
                res[i].type   = TOKEN_OPERATOR;
                res[i].symbol = '*';
                i++;
            }

            if (res[i-1].type == TOKEN_VARIABLE && isalpha((unsigned char)*ptr)) {
                if (identifier_length(ptr) == 1 && is_variable_char(*ptr)) {
                    free(res);
                    return NULL;
                }
            }
        }
        if (isalpha(*ptr))
        {
            int len = identifier_length(ptr);
            if (len == 1 && is_variable_char(*ptr))
            {
                res[i].type   = TOKEN_VARIABLE;
                res[i].symbol = *ptr;
                i++;
                ptr++;
                continue;
            }

            if (is_supported_function(ptr, len))
            {
                res[i].type = TOKEN_FUNCTION;
                memcpy(res[i].funcName, ptr, (size_t)len);
                res[i].funcName[len] = '\0';
                i++;
                ptr += len;
                continue;
            }

            free(res);
            return NULL;
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
