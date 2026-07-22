// Regression: fill_default_args() (src/vm/internal/base/interpret.cc)
// evaluates a defaulted parameter's value by calling a closure, via
//     push_svalue(call_function_pointer(sv_funcp.u.fp, 0));
// push_svalue(x) is the macro `STACK_INC; assign_svalue_no_free(sp, x);`, so
// STACK_INC ran BEFORE the closure was even evaluated. If that closure
// error()s (an ordinary thing for a default-argument expression to do), the
// throw unwound past a half-initialized stack slot AND skipped the trailing
// ref-free and fp restore -- leaving the VM eval stack corrupt, so the next
// error-unwind pop_n_elems() freed a garbage slot and crashed. The result is
// now computed into a local before being pushed, and the ref-free + fp
// restore are DEFER'd so they run on the throw path too. A caught error() in
// a default-argument expression must leave the VM stack intact, so ordinary
// work after the catch keeps running. Reachable from every default-fill call
// path: direct calls, ::inherited() calls, and (: foo :) function pointers.

int direct(int a: (: error("boom\n") :)) { return a; }

int guard_a, guard_b, guard_c;

void do_tests() {
  object child;
  mixed err;

  // Sibling guard locals: a corrupted eval stack tended to clobber these.
  guard_a = 0x1111;
  guard_b = 0x2222;
  guard_c = 0x3333;

  // 1. direct call (F_CALL_FUNCTION_BY_ADDRESS)
  err = catch(direct());
  ASSERT2(stringp(err), "expected the default expr's error() to propagate");

  // 2. function-pointer call (FP_LOCAL)
  err = catch(evaluate((: direct :)));
  ASSERT2(stringp(err), "expected error() through a (: :) fp too");

  // 2b. EXTERNAL call (call_other) -- routes through apply_low() in apply.cc,
  // which has its OWN inline default-args fill loop separate from
  // interpret.cc's fill_default_args(). This is the ordinary ob->foo() shape.
  err = catch(this_object()->direct());
  ASSERT2(stringp(err), "expected error() through apply_low (call_other) default fill");

  // 3. inherited call (F_CALL_INHERITED) -- build a parent with the erroring
  // default and a child that reaches it via ::parent_fn().
  rm("/gen_defarg_parent.c");
  write_file("/gen_defarg_parent.c",
    "int parent_fn(int a: (: error(\"boom\\n\") :)) { return a; }\n");
  rm("/gen_defarg_child.c");
  write_file("/gen_defarg_child.c",
    "inherit \"/gen_defarg_parent\";\n"
    "int call_it() { return ::parent_fn(); }\n");
  child = load_object("/gen_defarg_child");
  ASSERT(objectp(child));
  err = catch(child->call_it());
  ASSERT2(stringp(err), "expected error() through ::inherited() default fill");
  destruct(child);
  rm("/gen_defarg_child.c");
  rm("/gen_defarg_parent.c");

  // The VM stack must be intact after all three unwinds: guards unclobbered
  // and ordinary evaluation still correct.
  ASSERT_EQ(0x1111, guard_a);
  ASSERT_EQ(0x2222, guard_b);
  ASSERT_EQ(0x3333, guard_c);
  ASSERT_EQ(6, 1 + 2 + 3);

  // And a well-behaved default (no error()) still fills correctly.
  ASSERT_EQ(42, direct(42));
}
