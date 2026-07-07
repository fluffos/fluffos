// in LPC, there are no such thing as boolean value. LPC conditionals is really just comparing value to 0.
// Previously, only 0 (int) is false in conditionals, yet 0.0 == 0 would return 1 instead, causing confusion.
//
// This has now been fixed, so that floats with an 0 value would also evaluate to false.

int logical_assign_counter;

mixed counted(mixed value) {
  logical_assign_counter++;
  return value;
}

void do_tests() {
  int tmp = 1;
  mapping m;
  int logic_var;
  mixed maybe;

  tmp = 1;
  if(0.0) {
    tmp = 0;
  }
  ASSERT_EQ(1, tmp);

  tmp = 1;
  if(0.0000000000000) {
    tmp = 0;
  }
  ASSERT_EQ(1, tmp);

  tmp = 1;
  if (0.0 != 0) {
    tmp = 0;
  }
  ASSERT_EQ(1, tmp);

  tmp = 1;
  tmp = 0.0 ? 0 : 1;
  ASSERT_EQ(1, tmp);

  tmp = 1;
  tmp = 0.0000000 ? 0 : 1;
  ASSERT_EQ(1, tmp);

  tmp = 1;
  do {
    tmp++;
    if(tmp >= 3) break;
  } while(0.0);
  ASSERT_EQ(2, tmp);

  tmp = 1;
  do {
    tmp++;
    if(tmp >= 3) break;
  } while(0.0000000000000);
  ASSERT_EQ(2, tmp);

  // this will never be true since they are compared by type.
  ASSERT_NE(0.0, 0);
  ASSERT_NE(0.00000000000000, 0);

  // https://github.com/fluffos/fluffos/issues/889
  // 0.0 should be considered falsy in logical operations
  ASSERT_EQ(1, 0.0 || 1);
  ASSERT_EQ(1, 1 || 0.0);
  ASSERT_EQ(0.0, 0.0 && 1);
  ASSERT_EQ(0.0, 1 && 0.0);

  // Test ?? (nullish coalescing) operator
  // Only undefined triggers fallback, not falsy values
  m = ([]);

  // undefined mapping key should use fallback
  ASSERT_EQ("default", m["nonexistent"] ?? "default");

  // 0 should NOT use fallback (only undefined does)
  ASSERT_EQ(0, 0 ?? 42);
  ASSERT_EQ(0.0, 0.0 ?? 3.14);

  // empty string should NOT use fallback
  ASSERT_EQ("", "" ?? "fallback");

  // empty array should NOT use fallback
  ASSERT_EQ(({ }), ({ }) ?? ({ 1, 2, 3 }));

  // defined mapping value (even if 0) should NOT use fallback
  m["key"] = 0;
  ASSERT_EQ(0, m["key"] ?? 99);

  // defined mapping value should be returned
  m["name"] = "Alice";
  ASSERT_EQ("Alice", m["name"] ?? "default");

  // Difference between || and ??
  ASSERT_EQ(42, 0 || 42);       // || treats 0 as falsy
  ASSERT_EQ(0, 0 ?? 42);        // ?? only treats undefined as trigger
  ASSERT_EQ("", "" || "yes"); // || treats "" as truthy
  ASSERT_EQ("yes", ([])[0] || "yes"); // || treats undefined as falsy
  ASSERT_EQ("", "" ?? "yes");   // ?? does not treat "" as trigger

  // Logical assignment operators (||=, &&=, ??=)
  logical_assign_counter = 0;
  logic_var = 0;
  logic_var ||= counted(7);
  ASSERT_EQ(7, logic_var);
  ASSERT_EQ(1, logical_assign_counter);

  logical_assign_counter = 0;
  logic_var ||= counted(9);
  ASSERT_EQ(7, logic_var);
  ASSERT_EQ(0, logical_assign_counter);

  logical_assign_counter = 0;
  logic_var = 2;
  logic_var &&= counted(5);
  ASSERT_EQ(5, logic_var);
  ASSERT_EQ(1, logical_assign_counter);

  logical_assign_counter = 0;
  logic_var = 0;
  logic_var &&= counted(8);
  ASSERT_EQ(0, logic_var);
  ASSERT_EQ(0, logical_assign_counter);

  logical_assign_counter = 0;
  m = ([]);
  m["missing"] ??= counted("fallback");
  ASSERT_EQ("fallback", m["missing"]);
  ASSERT_EQ(1, logical_assign_counter);

  logical_assign_counter = 0;
  m["missing"] ??= counted("ignored");
  ASSERT_EQ("fallback", m["missing"]);
  ASSERT_EQ(0, logical_assign_counter);

  logical_assign_counter = 0;
  maybe = m["other"];
  maybe ??= counted(42);
  ASSERT_EQ(42, maybe);
  ASSERT_EQ(1, logical_assign_counter);

  logical_assign_counter = 0;
  maybe ??= counted(100);
  ASSERT_EQ(42, maybe);
  ASSERT_EQ(0, logical_assign_counter);

  // Test ternary operator without spaces (issue #1170)
  // These should all work without requiring spaces around ? and :
  string icon;
  icon = (1)?"yes":"no";
  ASSERT_EQ("yes", icon);

  icon = (0)?"yes":"no";
  ASSERT_EQ("no", icon);

  // Nested ternary without spaces
  icon = (1)?"a":(0)?"b":"c";
  ASSERT_EQ("a", icon);

  icon = (0)?"a":(1)?"b":"c";
  ASSERT_EQ("b", icon);

  icon = (0)?"a":(0)?"b":"c";
  ASSERT_EQ("c", icon);

  // Mixed with and without spaces
  icon = (1) ? "yes":"no";
  ASSERT_EQ("yes", icon);

  icon = (1)?"yes" : "no";
  ASSERT_EQ("yes", icon);

  // Test with other single-char tokens before ?
  int num = 5;
  icon = num>3?"big":"small";
  ASSERT_EQ("big", icon);

  icon = (num)?"yes":"no";
  ASSERT_EQ("yes", icon);

  icon = ({1,2,3})[0]?"yes":"no";
  ASSERT_EQ("yes", icon);
}
