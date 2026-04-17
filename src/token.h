#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,     // 3.14, 5
    TOKEN_OPERATOR,   // +, -, *, /, ^
    TOKEN_VARIABLE,   // x
    TOKEN_FUNCTION,   // sin, cos
    TOKEN_LPAREN,     // (
    TOKEN_RPAREN      // )
} TokenType;

typedef struct {
    TokenType type;
    double value;       // only for TOKEN_NUMBER
    char symbol;        // for operator ('+') or variable ('x')
    char funcName[8];   // for function names like "sqrt"
} Token;

Token *tokenizer(char *input);

#endif
