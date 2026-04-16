#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "token.h"
#include "parser.h"

// Mirrors the pole-detection logic inside draw_function (graph.c).
// Must be kept in sync with that function.
static bool segment_is_pole(Token *tokens, float x0, float x1)
{
    double y0 = evaluate_postfix(tokens, x0);
    double y1 = evaluate_postfix(tokens, x1);
    if (isnan(y0) || isnan(y1)) return false;   // outer isnan check handles this
    if (y0 * y1 >= 0) return false;             // same sign → no divergence

    double y_mid = evaluate_postfix(tokens, (x0 + x1) * 0.5f);
    double lo = y0 < y1 ? y0 : y1;
    double hi = y0 < y1 ? y1 : y0;
    return isnan(y_mid) || y_mid < lo || y_mid > hi;
}

static void assert_pole(const char *expr, float x0, float x1,
                        bool expected, const char *name)
{
    Token *tokens = parse((char *)expr);
    if (!tokens) {
        printf("❌ [FAILED] %s (parse returned NULL)\n", name);
        exit(1);
    }
    bool got = segment_is_pole(tokens, x0, x1);
    free(tokens);
    if (got != expected) {
        printf("❌ [FAILED] %s: expected %s but got %s\n",
               name,
               expected ? "POLE"     : "CONTINUOUS",
               got      ? "POLE"     : "CONTINUOUS");
        exit(1);
    }
    printf("✅ [PASSED] %s\n", name);
}

int main(void)
{
    printf("=== RUNNING GRAPH SEGMENT TESTS ===\n\n");

    // ---- 1/x: pole at x=0 ----
    // Any step straddling x=0 should be detected regardless of step size or offset.
    assert_pole("1/x", -0.245f,  0.005f,  true,  "1/x: narrow step straddles x=0 (mid not between)");
    assert_pole("1/x", -0.5f,    0.5f,    true,  "1/x: step=1, mid=NaN at x=0");
    assert_pole("1/x", -1.0f,    1.0f,    true,  "1/x: wide step, mid=NaN at x=0");
    assert_pole("1/x", -10.0f,   10.0f,   true,  "1/x: very wide step, mid jumps outside range");

    // 1/x segments that do NOT cross the pole should be drawn normally
    assert_pole("1/x",  0.5f,    0.75f,   false, "1/x: both on positive side");
    assert_pole("1/x", -0.5f,   -0.25f,   false, "1/x: both on negative side");
    assert_pole("1/x",  1.0f,    2.0f,    false, "1/x: far from pole");

    // ---- Legitimate zero crossings: must NOT be treated as poles ----
    assert_pole("x",         -0.25f,  0.25f,  false, "x: zero crossing, mid=0 is between");
    assert_pole("x^2 - 4",  -2.5f,  -1.5f,   false, "x^2-4: zero crossing at x=-2");
    assert_pole("x^2 - 4",   1.5f,   2.5f,   false, "x^2-4: zero crossing at x=2");
    assert_pole("x^2 - 1",  -1.5f,  -0.5f,   false, "x^2-1: zero crossing at x=-1");
    assert_pole("x^2 - 1",   0.5f,   1.5f,   false, "x^2-1: zero crossing at x=1");

    printf("\n=== ALL GRAPH TESTS PASSED ===\n");
    return 0;
}
