#ifndef PARSER_H
#define PARSER_H

#include "token.h"

int get_precedence(char op);
Token* infix_to_postfix(Token* infix);
double evaluate_postfix(Token* postfix, double x_value);
Token * parse(char *input);


#endif