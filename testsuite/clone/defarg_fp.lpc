// Fixture for tests/compiler/default_args_inherit.lpc: defaults through a
// local function pointer.
int foo(int a: (: 42 :)) { return a; }
int go_fp() { function f = (: foo :); return f(); }
int go_fp2() { return evaluate((: foo :)); }
