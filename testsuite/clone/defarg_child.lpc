// Fixture for tests/compiler/default_args_inherit.lpc: overrides an
// inherited function that has a default argument, SAME parameter name.
inherit "/clone/defarg_parent";
int foo(int a: (: 2 :)) { return a + 10; }
int go() { return foo(); }
int gop() { return ::foo(); }
