// Removes every occurrence of `val` from the array referenced by `arr`,
// writing the shrunken array back through the reference on each pass. This
// mirrors the eject_value()/shift()/pop() reassignment pattern that exercises
// a `ref` taken into a mapping element.
private void eject_all(mixed ref *arr, mixed val) {
  int i;

  while((i = member_array(val, arr)) != -1)
    arr = arr[0..i - 1] + arr[i + 1..];
}

void do_tests() {
  int x = 100;
  string* tmp1 = ({ "x", "y", "z" });
  mapping tmp = ([ "ab": 10, 20: "ab", tmp1: "abc" ]);
  string str = "(ab)"; // String to run through sscanf()
  string fail = "";
  mapping roles_map;
  mapping nested_map;

  ASSERT_EQ(10, tmp["ab"]);
  ASSERT_EQ(10, tmp["a" + "b"]);
  ASSERT_EQ(10, tmp[tmp[20]]);

  sscanf(str, "(%s)", fail);
  ASSERT_EQ("ab", fail);
  ASSERT_EQ(10, tmp[fail]);


  ASSERT_EQ("ab", tmp[20]);
  ASSERT_EQ("ab", tmp[10 * 2]);
  ASSERT_EQ("ab", tmp[x/5]);

  // Taking a `ref` into a mapping element whose value is not itself a mapping
  // (here, an array) used to compute the wrong keep-alive owner in
  // push_indexed_lvalue(): it captured the indexed value instead of the
  // containing mapping, tagged it T_MAPPING, and later freed the array as a
  // mapping in kill_ref() -> a type-confusion crash. The reference must mutate
  // the final element and leave the rest of the mapping intact.
  roles_map = ([ "roles": ({ "a", "b", "a", "c", "a" }), "keep": 1 ]);
  eject_all(ref roles_map["roles"], "a");
  ASSERT_EQ(({ "b", "c" }), roles_map["roles"]);
  ASSERT_EQ(1, roles_map["keep"]);

  // Same path one level deeper: ref into a mapping nested inside a mapping.
  nested_map = ([ "grp": ([ "roles": ({ "x", "y", "x" }) ]) ]);
  eject_all(ref nested_map["grp"]["roles"], "x");
  ASSERT_EQ(({ "y" }), nested_map["grp"]["roles"]);
}
