// has_cycle(mixed): 1 if the value's reference graph contains a loop.
// The walker is iterative, so unlike save_variable()/copy() it has no
// depth cap -- deep acyclic nesting must scan clean, not error.

class Node {
  mixed data;
  mixed next;
}

private void run_checks() {
#if efun_defined(has_cycle)
  mixed *a, *deep, *shared;
  mapping m1, m2;
  class Node n1, n2;
  function f;
  int i;

  // leaves and acyclic structures
  ASSERT_EQ(0, has_cycle(0));
  ASSERT_EQ(0, has_cycle("string"));
  ASSERT_EQ(0, has_cycle(({ 1, ({ 2, ({ 3 }) }), ([ "k": ({ 4 }) ]) })));

  // DAG sharing (the same array referenced twice) is NOT a cycle
  shared = ({ 1 });
  ASSERT_EQ(0, has_cycle(({ shared, shared, ([ "k": shared ]) })));

  // self-referential array, directly and one level down
  a = ({ 0 });
  a[0] = a;
  ASSERT_EQ(1, has_cycle(a));
  ASSERT_EQ(1, has_cycle(({ "wrapper", a })));
  a[0] = 0;
  ASSERT_EQ(0, has_cycle(a));

  // mutual mapping cycle
  m1 = ([]);
  m2 = ([]);
  m1["peer"] = m2;
  m2["peer"] = m1;
  ASSERT_EQ(1, has_cycle(m1));
  ASSERT_EQ(1, has_cycle(m2));
  map_delete(m1, "peer");
  ASSERT_EQ(0, has_cycle(m1));
  ASSERT_EQ(0, has_cycle(m2));

  // cycle where a mapping is used as a KEY of another mapping
  m1 = ([]);
  m2 = ([]);
  m1[m2] = 1;
  m2[m1] = 1;
  ASSERT_EQ(1, has_cycle(m1));
  map_delete(m1, m2);
  ASSERT_EQ(0, has_cycle(m2));
  map_delete(m2, m1);

  // circular class list
  n1 = new(class Node, data: 1);
  n2 = new(class Node, data: 2);
  n1.next = n2;
  n2.next = n1;
  ASSERT_EQ(1, has_cycle(n1));
  n2.next = 0;
  ASSERT_EQ(0, has_cycle(n1));

  // cycle through a function pointer's captured arguments
  a = ({ 0 });
  f = (: member_array, 0, a :);
  a[0] = f;
  ASSERT_EQ(1, has_cycle(a));
  ASSERT_EQ(1, has_cycle(f));
  a[0] = 0;
  ASSERT_EQ(0, has_cycle(f));

  // deep acyclic nesting far beyond MAX_SAVE_SVALUE_DEPTH: the iterative
  // walker must return 0 cleanly where save_variable() errors out
  deep = ({ 0 });
  for (i = 0; i < 5000; i++) {
    deep = ({ deep });
  }
  ASSERT_EQ(0, has_cycle(deep));
  ASSERT(catch(save_variable(deep)));
#endif
  ASSERT(1);
}

void do_tests() {
  // unconditional teardown (AGENTS.md section 7): if anything above errors
  // uncaught mid-test, locally-built cycles unwind into orphans that the
  // harness's post-file check_memory() now hard-fails on -- collect them
  // so one regression stays one [ FAILED ] entry instead of aborting the
  // whole randomized run
  string err = catch(run_checks());
#if efun_defined(find_orphaned_cycles)
  find_orphaned_cycles(1);
#endif
  if (err) error(err);
}
