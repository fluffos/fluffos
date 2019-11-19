int same_array(mixed *x, mixed *y) {
  if (!arrayp(x) || !arrayp(y)) return 0;
  if (sizeof(x) != sizeof(y)) return 0;
  for (int i = 0; i < sizeof(x); i++) {
    if (arrayp(x[i])) {
      if (!same_array(x[i], y[i]))
        return 0;
    } else if (x[i] != y[i])
      return 0;
  }
  return 1;
}

void do_tests() {
#ifdef __PACKAGE_PCRE__
  pcre_match("123", "^[0-9]+$"); // or other pattern
  ASSERT(same_array(
    pcre_assoc("testhahatest", ({ "haha", "te" }), ({ 2, 3 }), 4),
    ({ ({ "", "te", "st", "haha", "", "te", "st" }),
        ({ 4, 3, 4, 2, 4, 3, 4 }) })));
  ASSERT(catch(pcre_assoc("foo", ({ 1 }), ({ 2, 3 }))));
  ASSERT(catch(pcre_assoc("foo", ({ 1, 2 }), ({ 2, 3 }))));
  ASSERT(catch(pcre_assoc("foo", ({ "bar", "+" }), ({ 0, 1 }))));
#else
  write("PACKAGE_PCRE not defined, test skipped.");
#endif
}
