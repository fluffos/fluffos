// Locks the "(:" functional forms after 9.2 moved the named-vs-anonymous
// split from the lexer's one-byte peek into grammar productions: every
// form must keep its legacy meaning.
#include <tests.h>

int helper(int x) { return x * 3; }
int gvar = 7;

void do_tests() {
    // Named local function reference.
    ASSERT_EQ(15, evaluate((: helper :), 5));
    // Named efun reference.
    ASSERT_EQ(2, evaluate((: sizeof :), ({ 1, 2 })));
    // Partial application (the ',' lookahead after the name).
    ASSERT_EQ(12, evaluate((: helper, 4 :)));
    // efun:: override form.
    ASSERT_EQ(3, evaluate((: efun::sizeof :), ({ 9, 9, 9 })));
    // Anonymous $N body.
    ASSERT_EQ(30, evaluate((: $1 + $2 :), 10, 20));
    // Global variable functional.
    ASSERT_EQ(7, evaluate((: gvar :)));
    // Anonymous body that STARTS with a defined name (the '(' lookahead
    // must fall through to the expression body, not the named form).
    ASSERT_EQ(7, evaluate((: helper(2) + 1 :)));
    // Nested functionals.
    ASSERT_EQ(101, evaluate(evaluate((: (: $1 + 100 :) :)), 1));
    // Comment between "(:" and the name flows through the ordinary
    // comment rules now (no dedicated lexer state).
    ASSERT_EQ(9, evaluate((: /* named */ helper :), 3));
}
