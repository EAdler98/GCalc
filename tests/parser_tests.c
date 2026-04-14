#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "parser.h"

// פונקציית האימות (Assert)
void assert_postfix(Token *expected, Token *actual, const char *test_name)
{
    int i = 0;

    // רצים בלולאה כל עוד לא הגענו לסוף של אחד המערכים
    while (expected[i].type != TOKEN_EOF && actual[i].type != TOKEN_EOF)
    {
        // 1. בדיקת סוג הטוקן
        if (expected[i].type != actual[i].type)
        {
            printf("❌ [FAILED] %s\n", test_name);
            printf("   At index %d: Expected Type %d, but got Type %d\n", i, expected[i].type, actual[i].type);
            exit(1); // <--- משגר כישלון לגיטהאב!
        }

        // 2. בדיקת הערך (אם זה מספר)
        if (expected[i].type == TOKEN_NUMBER)
        {
            if (expected[i].value != actual[i].value)
            {
                printf("❌ [FAILED] %s\n", test_name);
                printf("   At index %d: Expected Number %g, but got %g\n", i, expected[i].value, actual[i].value);
                exit(1);
            }
        }

        // 3. בדיקת הסימן (אם זה אופרטור או משתנה)
        if (expected[i].type == TOKEN_OPERATOR || expected[i].type == TOKEN_VARIABLE)
        {
            if (expected[i].symbol != actual[i].symbol)
            {
                printf("❌ [FAILED] %s\n", test_name);
                printf("   At index %d: Expected Symbol '%c', but got '%c'\n", i, expected[i].symbol, actual[i].symbol);
                exit(1);
            }
        }

        i++;
    }

    // 4. מוודאים ששני המערכים הסתיימו באותו זמן (שאחד לא ארוך מהשני)
    if (expected[i].type != actual[i].type)
    {
        printf("❌ [FAILED] %s\n", test_name);
        printf("   Length mismatch! One array ended before the other.\n");
        exit(1);
    }

    // אם הגענו לפה, הכל תקין!
    printf("✅ [PASSED] %s\n", test_name);
}
// פונקציית עזר להרצת טסט מלא (ממחרוזת ועד אימות)
void run_test(char *input, Token *expected, const char *test_name)
{
    Token *infix = tokenizer(input);
    if (!infix)
    {
        printf("❌ [FAILED] %s (Tokenizer returned NULL)\n", test_name);
        exit(1);
    }

    Token *actual_postfix = infix_to_postfix(infix);
    if (!actual_postfix)
    {
        printf("❌ [FAILED] %s (Parser returned NULL)\n", test_name);
        exit(1);
    }

    // הרצת האימות!
    assert_postfix(expected, actual_postfix, test_name);

    free(infix);
    free(actual_postfix);
}

int main()
{
    printf("=== RUNNING AUTOMATED PARSER TESTS ===\n\n");

    // ---------------------------------------------------------
    // טסט 1: סדר פעולות פשוט (3 + 4 * x)
    // התשובה המצופה: 3, 4, x, *, +
    // ---------------------------------------------------------
    Token expected1[] = {
        {TOKEN_NUMBER, 3.0, 0},
        {TOKEN_NUMBER, 4.0, 0},
        {TOKEN_VARIABLE, 0.0, 'x'},
        {TOKEN_OPERATOR, 0.0, '*'},
        {TOKEN_OPERATOR, 0.0, '+'},
        {TOKEN_EOF, 0.0, 0} // חובה לסגור עם EOF!
    };
    run_test("3 + 4 * x", expected1, "Basic Precedence (3 + 4 * x)");

    // ---------------------------------------------------------
    // טסט 2: חזקה (x ^ 2 * 5)
    // התשובה המצופה: x, 2, ^, 5, *
    // ---------------------------------------------------------
    Token expected2[] = {
        {TOKEN_VARIABLE, 0.0, 'x'},
        {TOKEN_NUMBER, 2.0, 0},
        {TOKEN_OPERATOR, 0.0, '^'},
        {TOKEN_NUMBER, 5.0, 0},
        {TOKEN_OPERATOR, 0.0, '*'},
        {TOKEN_EOF, 0.0, 0}};
    run_test("x ^ 2 * 5", expected2, "Power Operator (x ^ 2 * 5)");
    // ---------------------------------------------------------
    // טסט 3: משוואה ריבועית עם כפל סמוי (x^2 + 2x - 3)
    // הלקסר יתרגם ל: x ^ 2 + 2 * x - 3
    // התשובה המצופה בפוסטפיקס: x 2 ^ 2 x * + 3 -
    // ---------------------------------------------------------
    Token expected_quad[] = {
        {TOKEN_VARIABLE, 0.0, 'x'},
        {TOKEN_NUMBER, 2.0, 0},
        {TOKEN_OPERATOR, 0.0, '^'},
        {TOKEN_NUMBER, 2.0, 0},
        {TOKEN_VARIABLE, 0.0, 'x'},
        {TOKEN_OPERATOR, 0.0, '*'}, // הכפל הסמוי שהלקסר הוסיף!
        {TOKEN_OPERATOR, 0.0, '+'},
        {TOKEN_NUMBER, 3.0, 0},
        {TOKEN_OPERATOR, 0.0, '-'},
        {TOKEN_EOF, 0.0, 0}};
    run_test("x^2 + 2x - 3", expected_quad, "Quadratic Equation (x^2 + 2x - 3)");

    // ---------------------------------------------------------
    // טסט 4: כפל סמוי עם סוגריים ( 2(x+1) )
    // הלקסר יתרגם ל: 2 * ( x + 1 )
    // התשובה המצופה בפוסטפיקס: 2 x 1 + *
    // ---------------------------------------------------------
    Token expected_parens[] = {
        {TOKEN_NUMBER, 2.0, 0},
        {TOKEN_VARIABLE, 0.0, 'x'},
        {TOKEN_NUMBER, 1.0, 0},
        {TOKEN_OPERATOR, 0.0, '+'},
        {TOKEN_OPERATOR, 0.0, '*'},
        {TOKEN_EOF, 0.0, 0}};
    run_test("2(x+1)", expected_parens, "Implicit Mult with Parentheses 2(x+1)");

    // ---------------------------------------------------------
    // טסט 5: חלוקה וסדר פעולות מורכב ( 10 / 2 * 5 )
    // שים לב! כפל וחילוק הם באותה רמת קדימות (Precedence),
    // לכן החישוב הוא משמאל לימין (קודם חילוק, ואז כפל).
    // התשובה המצופה: 10 2 / 5 * (ולא 10 2 5 * / !)
    // ---------------------------------------------------------
    Token expected_div_mult[] = {
        {TOKEN_NUMBER, 10.0, 0},
        {TOKEN_NUMBER, 2.0, 0},
        {TOKEN_OPERATOR, 0.0, '/'},
        {TOKEN_NUMBER, 5.0, 0},
        {TOKEN_OPERATOR, 0.0, '*'},
        {TOKEN_EOF, 0.0, 0}};
    run_test("10 / 2 * 5", expected_div_mult, "Left-to-Right Associativity (10 / 2 * 5)");

    // ... (כאן אפשר להוסיף עוד ועוד טסטים) ...

    printf("\n=== ALL TESTS PASSED SUCCESSFULLY! ===\n");

    // החזרת 0 מסמנת ל-GitHub Actions שהכל עבר בשלום (וי ירוק!)
    return 0;
}