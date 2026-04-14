#ifndef TOKEN_H
#define TOKEN_H
#include "raylib.h"

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
    char funcName[3];   // for function ("sin")
} Token;

typedef struct {
    Token * tokens;
    Color color;       
} Function;


Token *tokenizer(char *input);

#endif
