// foreach (key, ref value in mapping) aims the ref at the mapping node's
// value slot. Deleting the iterated key used to send that node straight to
// the global free list (the loop never set MAP_LOCKED), so the very next
// mapping insert anywhere recycled it -- and the loop's write through the
// ref then landed in the OTHER mapping's value slot. The loop now locks the
// mapping like f(ref m[key]) does, deferring node frees until the ref dies.

void do_tests() {
  mapping m, other;
  string *seen = ({});

  // Basic in-place mutation through the ref still works.
  m = ([ "a": 1, "b": 2 ]);
  foreach (string k, mixed ref v in m) {
    v = v * 10;
  }
  ASSERT_EQ(10, m["a"]);
  ASSERT_EQ(20, m["b"]);

  // Deleting the iterated key must not let writes through the ref corrupt
  // an unrelated mapping that recycles the node.
  m = ([ "a": 1, "b": 2 ]);
  other = ([]);
  foreach (string k, mixed ref v in m) {
    seen += ({ k });
    map_delete(m, k);
    other["x" + k] = 42;
    v = 99;
  }
  ASSERT_EQ(2, sizeof(seen));
  ASSERT_EQ(0, sizeof(m));
  ASSERT_EQ(2, sizeof(other));
  foreach (string k2, mixed val in other) {
    ASSERT_EQ(42, val);
  }
}
