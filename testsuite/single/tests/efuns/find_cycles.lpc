// find_cycles(mixed): index paths (one string per back-edge) locating every
// slot that closes a reference loop. Array/class paths are deterministic;
// mapping paths depend on internal table order, so those are checked by
// count and prefix only.

class Node {
  mixed data;
  mixed next;
}

private void run_checks() {
#if efun_defined(find_cycles)
  mixed *a, *b, *two;
  mapping m;
  class Node n;
  string *paths;

  // acyclic values report nothing
  ASSERT_EQ(({}), find_cycles(0));
  ASSERT_EQ(({}), find_cycles(({ 1, ({ 2 }) })));

  // self-referential array: the back-edge is the slot itself
  a = ({ "x", 0 });
  a[1] = a;
  ASSERT_EQ(({ "[1]" }), find_cycles(a));
  a[1] = 0;   // break before reassigning, or the old array is leaked

  // nested: path walks through the outer array
  b = ({ ({ 0 }) });
  b[0][0] = b;
  ASSERT_EQ(({ "[0][0]" }), find_cycles(b));
  b[0][0] = 0;

  // two independent loops: one path each
  a = ({ 0 });
  a[0] = a;
  b = ({ 0 });
  b[0] = b;
  two = ({ a, b });
  ASSERT_EQ(({ "[0][0]", "[1][0]" }), find_cycles(two));

  // class ring: field index in declaration order (next == field 1)
  n = new(class Node, data: 1);
  n.next = n;
  ASSERT_EQ(({ ".1" }), find_cycles(n));

  // mapping value cycle: exactly one back-edge, key rendered in the path
  m = ([]);
  m["self"] = m;
  paths = find_cycles(m);
  ASSERT_EQ(({ "[\"self\"]" }), paths);

  // break everything before returning
  a[0] = 0;
  b[0] = 0;
  n.next = 0;
  map_delete(m, "self");
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
