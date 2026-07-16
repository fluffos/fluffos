// break_cycles(mixed): clears every reference loop in the value, in place,
// by zeroing the back-edge slots (deleting the node when the back-edge is a
// mapping KEY). Returns the number of edges broken. Everything that is not
// part of a loop -- including DAG sharing -- must be left untouched, and the
// value must afterwards be save/copy/print-safe again.

class Node {
  mixed data;
  mixed next;
}

private void run_checks() {
#if efun_defined(break_cycles)
  mixed *a, *ring1, *ring2, *ring3, *shared, *dag;
  mapping m1, m2;
  class Node n1, n2;
  function f, g;
  object clone;

  // non-compound and acyclic values: nothing to do
  ASSERT_EQ(0, break_cycles(42));
  ASSERT_EQ(0, break_cycles(({ 1, ({ 2 }), ([ "k": 3 ]) })));

  // self-referential array
  a = ({ "keep", 0 });
  a[1] = a;
  ASSERT_EQ(1, break_cycles(a));
  ASSERT_EQ("keep", a[0]);            // non-loop content untouched
  ASSERT_EQ(0, a[1]);                 // the back-edge is now 0
  ASSERT_EQ(0, has_cycle(a));
#if efun_defined(refs)
  ASSERT_EQ(1, refs(a));              // only the local variable holds it
#endif
  ASSERT_EQ(0, catch(save_variable(a)));  // saving works again
  ASSERT_EQ(0, catch(copy(a)));           // deep copy works again

  // DAG sharing is preserved, not treated as a loop
  shared = ({ 1 });
  dag = ({ shared, shared });
  ASSERT_EQ(0, break_cycles(dag));
  ASSERT(dag[0] == shared && dag[1] == shared);   // identity intact

  // mutual mapping cycle: exactly one edge cut, the rest intact
  m1 = ([ "name": "m1" ]);
  m2 = ([ "name": "m2" ]);
  m1["peer"] = m2;
  m2["peer"] = m1;
  ASSERT_EQ(1, break_cycles(m1));
  ASSERT_EQ(0, has_cycle(m1));
  ASSERT_EQ("m1", m1["name"]);
  ASSERT_EQ("m2", m2["name"]);
  // one of the two peer edges survives, the other is 0
  ASSERT((m1["peer"] == m2 && !m2["peer"]) || (m2["peer"] == m1 && !m1["peer"]));

  // ring of three arrays: one cut un-loops the whole ring
  ring1 = ({ 0 });
  ring2 = ({ 0 });
  ring3 = ({ 0 });
  ring1[0] = ring2;
  ring2[0] = ring3;
  ring3[0] = ring1;
  ASSERT_EQ(1, break_cycles(ring1));
  ASSERT_EQ(0, has_cycle(ring1));

  // circular class list
  n1 = new(class Node, data: 1);
  n2 = new(class Node, data: 2);
  n1.next = n2;
  n2.next = n1;
  ASSERT_EQ(1, break_cycles(n1));
  ASSERT_EQ(0, has_cycle(n1));
  ASSERT(n1.next == n2);              // tree edge kept, back-edge cut
  ASSERT_EQ(0, n2.next);

  // cycle closed in mapping-KEY position: the node is deleted
  m1 = ([]);
  m2 = ([]);
  m1[m2] = "v1";
  m2[m1] = "v2";
  ASSERT_EQ(1, break_cycles(m1));
  ASSERT_EQ(0, has_cycle(m1));
  ASSERT_EQ(1, sizeof(m1) + sizeof(m2));  // exactly one node was deleted

  // cycle through a function pointer's captured arguments: the captured
  // slot inside the args list is zeroed, the funptr itself survives
  a = ({ 0 });
  f = (: member_array, 0, a :);
  a[0] = f;
  ASSERT_EQ(1, break_cycles(a));
  ASSERT_EQ(0, has_cycle(a));
  ASSERT(functionp(a[0]));

  // bind() to a DIFFERENT owner creates a new funptr that SHARES the old
  // one's args array, so a loop can close on the funptr->args edge itself
  // (no svalue slot to zero): the bound funptr's args list is detached and
  // replaced with a zero-filled one instead. Regression: this used to be
  // silently unbreakable (break_cycles() returned 0, has_cycle() stayed 1).
  // (bind() to the SAME owner is a no-op that returns the original funptr,
  // which is why the clone is needed to construct the shared-args pair.)
  a = ({ 0 });
  f = (: member_array, 0, a :);       // f.args = ({ 0, a })
  clone = new(__FILE__);
  g = bind(f, clone);                 // g is a new funptr sharing f's args
  ASSERT(g != f);
  a[0] = g;                           // args -> a -> g -> args : the loop
  ASSERT_EQ(1, has_cycle(f));
  ASSERT_EQ(1, break_cycles(f));
  ASSERT_EQ(0, has_cycle(f));
  ASSERT_EQ(0, has_cycle(g));
  ASSERT(functionp(a[0]));            // g itself survives, args detached
  destruct(clone);

  // values containing DESTRUCTED objects: walked as leaves and rendered
  // in paths without touching freed memory, freed normally on break
  clone = new(__FILE__);
  m1 = ([ clone: ({ 0 }) ]);
  m1[clone][0] = m1;                  // loop through the value under an object key
  destruct(clone);
  ASSERT_EQ(1, has_cycle(m1));
#if efun_defined(find_cycles)
  ASSERT_EQ(1, sizeof(find_cycles(m1)));  // renders the (dead) object key
#endif
  ASSERT_EQ(1, break_cycles(m1));
  ASSERT_EQ(0, has_cycle(m1));

  // idempotent: nothing left to break
  ASSERT_EQ(0, break_cycles(a));
  ASSERT_EQ(0, break_cycles(m1));
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
