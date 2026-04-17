#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "parser.h"

static void assert_postfix(Token *expected, Token *actual, const char *test_name)
{
    int i = 0;

    while (expected[i].type != TOKEN_EOF && actual[i].type != TOKEN_EOF)
    {
        if (expected[i].type != actual[i].type)
        {
            printf("[FAILED] %s\n", test_name);
            printf("  At index %d: expected type %d, got %d\n", i, expected[i].type, actual[i].type);
            exit(1);
        }

        if (expected[i].type == TOKEN_NUMBER)
        {
            if (fabs(expected[i].value - actual[i].value) > 1e-9)
            {
                printf("[FAILED] %s\n", test_name);
                printf("  At index %d: expected number %g, got %g\n", i, expected[i].value, actual[i].value);
                exit(1);
            }
        }

        if (expected[i].type == TOKEN_OPERATOR || expected[i].type == TOKEN_VARIABLE)
        {
            if (expected[i].symbol != actual[i].symbol)
            {
                printf("[FAILED] %s\n", test_name);
                printf("  At index %d: expected symbol '%c', got '%c'\n", i, expected[i].symbol, actual[i].symbol);
                exit(1);
            }
        }

        if (expected[i].type == TOKEN_FUNCTION)
        {
            if (strcmp(expected[i].funcName, actual[i].funcName) != 0)
            {
                printf("[FAILED] %s\n", test_name);
                printf("  At index %d: expected function \"%s\", got \"%s\"\n", i, expected[i].funcName, actual[i].funcName);
                exit(1);
            }
        }

        i++;
    }

    if (expected[i].type != actual[i].type)
    {
        printf("[FAILED] %s\n", test_name);
        printf("  Length mismatch between expected and actual postfix output\n");
        exit(1);
    }

    printf("[PASSED] %s\n", test_name);
}

static void run_test(char *input, Token *expected, const char *test_name)
{
    Token *infix = tokenizer(input);
    if (!infix)
    {
        printf("[FAILED] %s (tokenizer returned NULL)\n", test_name);
        exit(1);
    }

    Token *actual_postfix = infix_to_postfix(infix);
    if (!actual_postfix)
    {
        printf("[FAILED] %s (parser returned NULL)\n", test_name);
        free(infix);
        exit(1);
    }

    assert_postfix(expected, actual_postfix, test_name);

    free(infix);
    free(actual_postfix);
}

static void assert_tokenizer_null(char *input, const char *test_name)
{
    Token *infix = tokenizer(input);
    if (infix != NULL)
    {
        printf("[FAILED] %s (tokenizer should have returned NULL)\n", test_name);
        free(infix);
        exit(1);
    }

    printf("[PASSED] %s\n", test_name);
}

static void assert_evaluate(const char *expr, double x, double expected, const char *test_name)
{
    Token *postfix = parse((char *)expr);
    if (!postfix)
    {
        printf("[FAILED] %s (parse returned NULL)\n", test_name);
        exit(1);
    }

    double result = evaluate_postfix(postfix, x);
    free(postfix);

    if (isnan(expected))
    {
        if (!isnan(result))
        {
            printf("[FAILED] %s: expected NAN but got %g\n", test_name, result);
            exit(1);
        }
        printf("[PASSED] %s\n", test_name);
        return;
    }

    if (fabs(result - expected) > 1e-9)
    {
        printf("[FAILED] %s: expected %g but got %g\n", test_name, expected, result);
        exit(1);
    }

    printf("[PASSED] %s\n", test_name);
}

int main(void)
{
    printf("=== RUNNING AUTOMATED PARSER TESTS ===\n\n");

    Token expected1[] = {
        {TOKEN_NUMBER, 3.0, 0, ""},
        {TOKEN_NUMBER, 4.0, 0, ""},
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_OPERATOR, 0.0, '*', ""},
        {TOKEN_OPERATOR, 0.0, '+', ""},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("3 + 4 * x", expected1, "Basic precedence (3 + 4 * x)");

    Token expected2[] = {
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_NUMBER, 2.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '^', ""},
        {TOKEN_NUMBER, 5.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '*', ""},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("x ^ 2 * 5", expected2, "Power operator (x ^ 2 * 5)");

    Token expected_quad[] = {
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_NUMBER, 2.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '^', ""},
        {TOKEN_NUMBER, 2.0, 0, ""},
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_OPERATOR, 0.0, '*', ""},
        {TOKEN_OPERATOR, 0.0, '+', ""},
        {TOKEN_NUMBER, 3.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '-', ""},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("x^2 + 2x - 3", expected_quad, "Quadratic equation (x^2 + 2x - 3)");

    Token expected_parens[] = {
        {TOKEN_NUMBER, 2.0, 0, ""},
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_NUMBER, 1.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '+', ""},
        {TOKEN_OPERATOR, 0.0, '*', ""},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("2(x+1)", expected_parens, "Implicit multiplication with parentheses");

    Token expected_div_mult[] = {
        {TOKEN_NUMBER, 10.0, 0, ""},
        {TOKEN_NUMBER, 2.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '/', ""},
        {TOKEN_NUMBER, 5.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '*', ""},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("10 / 2 * 5", expected_div_mult, "Left-to-right associativity");

    Token expected_unary[] = {
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_NUMBER, 2.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '^', ""},
        {TOKEN_OPERATOR, 0.0, 'n', ""},
        {TOKEN_NUMBER, 3.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '+', ""},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("-x^2 + 3", expected_unary, "Unary minus postfix shape");

    Token expected_pow_neg[] = {
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_NUMBER, 2.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, 'n', ""},
        {TOKEN_OPERATOR, 0.0, '^', ""},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("x^-2", expected_pow_neg, "Power with negative exponent");

    Token expected_sin[] = {
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_FUNCTION, 0.0, 0, "sin"},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("sin(x)", expected_sin, "Function call sin(x)");

    Token expected_sqrt[] = {
        {TOKEN_VARIABLE, 0.0, 'x', ""},
        {TOKEN_NUMBER, 1.0, 0, ""},
        {TOKEN_OPERATOR, 0.0, '+', ""},
        {TOKEN_FUNCTION, 0.0, 0, "sqrt"},
        {TOKEN_EOF, 0.0, 0, ""}
    };
    run_test("sqrt(x+1)", expected_sqrt, "Function call sqrt(x+1)");

    assert_tokenizer_null("xx", "Reject adjacent variables (xx)");
    assert_tokenizer_null("y", "Reject non-x variable (y)");
    assert_tokenizer_null("X", "Reject uppercase variable (X)");

    printf("\n=== RUNNING EVALUATE_POSTFIX TESTS ===\n\n");

    assert_evaluate("3 + 4", 0, 7.0, "Eval: addition");
    assert_evaluate("10 - 3", 0, 7.0, "Eval: subtraction");
    assert_evaluate("3 * 4", 0, 12.0, "Eval: multiplication");
    assert_evaluate("10 / 2", 0, 5.0, "Eval: division");
    assert_evaluate("2 ^ 3", 0, 8.0, "Eval: power");

    assert_evaluate("x", 3, 3.0, "Eval: variable x");
    assert_evaluate("x + 1", 5, 6.0, "Eval: x + 1 at x=5");
    assert_evaluate("2x", 5, 10.0, "Eval: implicit multiplication 2x at x=5");

    assert_evaluate("x^2 + 2x - 3", 1, 0.0, "Eval: quadratic root x=1");
    assert_evaluate("x^2 + 2x - 3", -3, 0.0, "Eval: quadratic root x=-3");
    assert_evaluate("x^2 + 2x - 3", 0, -3.0, "Eval: quadratic at x=0");
    assert_evaluate("x^2 + 2x - 3", 2, 5.0, "Eval: quadratic at x=2");

    assert_evaluate("1 / 0", 0, NAN, "Eval: division by zero -> NAN");
    assert_evaluate("x - 5", 2, -3.0, "Eval: negative result");
    assert_evaluate("x / 2", 3, 1.5, "Eval: fractional result");
    assert_evaluate("(x+1)(x-1)", 3, 8.0, "Eval: (x+1)(x-1) at x=3");

    assert_evaluate("-x^2 + 3", 2, -1.0, "Eval: unary minus -x^2+3 at x=2");
    assert_evaluate("-x^2 + 3", 0, 3.0, "Eval: unary minus -x^2+3 at x=0");
    assert_evaluate("-x^2 + 3", 1, 2.0, "Eval: unary minus -x^2+3 at x=1");

    assert_evaluate("x^-2", 2, 0.25, "Eval: x^-2 at x=2");
    assert_evaluate("x^-2", 4, 0.0625, "Eval: x^-2 at x=4");
    assert_evaluate("x^-2", 1, 1.0, "Eval: x^-2 at x=1");

    assert_evaluate("sqrt(9)", 0, 3.0, "Eval: sqrt(9)");
    assert_evaluate("sqrt(x+1)", 8, 3.0, "Eval: sqrt(x+1) at x=8");
    assert_evaluate("log(1)", 0, 0.0, "Eval: log(1)");
    assert_evaluate("sin(0)", 0, 0.0, "Eval: sin(0)");
    assert_evaluate("cos(0)", 0, 1.0, "Eval: cos(0)");
    assert_evaluate("sin(x)", 1.5707963267948966, 1.0, "Eval: sin(x) at x=pi/2");
    assert_evaluate("cos(x)", 3.141592653589793, -1.0, "Eval: cos(x) at x=pi");

    assert_evaluate("x^2 + 2*x - 3^", 2, NAN, "Eval: trailing operator -> NAN");
    assert_evaluate("x +", 1, NAN, "Eval: dangling + -> NAN");
    assert_evaluate("sqrt(-1)", 0, NAN, "Eval: sqrt negative -> NAN");
    assert_evaluate("log(0)", 0, NAN, "Eval: log zero -> NAN");

    printf("\n=== ALL TESTS PASSED SUCCESSFULLY! ===\n");
    return 0;
}
