mixed var = 0;

void do_tests() {
#ifndef __PACKAGE_CONTRIB__
  write("PACKAGE_CONTRIB not enabled, test did not run.\n");
#else
  string *values = ({
    0,
    MAX_INT,
    MIN_FLOAT,
    "test123",
    ({ "123", "345" }),
    ([ 1: ({"123"}), "123": 4 ]),
    this_object(),
  });
  object target;

  foreach(mixed v in values) {
    store_variable("var", v);
    ASSERT_EQ(var, v);
  }

  target = new(__FILE__);

  store_variable("var", 0);
  foreach(mixed v in values) {
    store_variable("var", v, target);
    ASSERT_EQ(fetch_variable("var", target), v);
    ASSERT_EQ(fetch_variable("var"), 0);
  }

  // variable doesn't exist will throw error
  ASSERT(catch(fetch_variable("not_exists")));
  ASSERT(catch(fetch_variable("not_exists", target)));

#endif
}
