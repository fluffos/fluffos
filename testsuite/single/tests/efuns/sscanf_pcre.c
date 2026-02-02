inherit "/inherit/tests";

void do_tests() {
#ifndef __PACKAGE_PCRE__
  write("PACKAGE_PCRE not defined, test skipped.");
  return;
#endif

  // __RC_SSCANF_USE_PCRE__ == CFG_INT(65) + RC_BASE_CONFIG_INT(256) == 321
  int old = get_config(321);
  set_config(321, 1);

  mixed err = catch {
    string a, b;
    int rc;

    rc = sscanf(":test hi", "%*([:/])%([\\w]+) %s", a, b);
    ASSERT_EQ(3, rc);
    ASSERT_EQ("test", a);
    ASSERT_EQ("hi", b);

    rc = sscanf("foo", "%([0-9]+)", a);
    ASSERT_EQ(0, rc);

    rc = sscanf("aba", "%((.)b\\1)", a);
    ASSERT_EQ(1, rc);
    ASSERT_EQ("aba", a);

    rc = sscanf("Hello hELLo", "%((?i)hello) %((?i)hello)", a, b);
    ASSERT_EQ(2, rc);
    ASSERT_EQ("Hello", a);
    ASSERT_EQ("hELLo", b);

    rc = sscanf("abc123", "%*([a-z]+)%([0-9]+)", a);
    ASSERT_EQ(2, rc);
    ASSERT_EQ("123", a);

    mixed tmp = catch(rc = sscanf("abba", "%(a(b)\\2a)", a));
    ASSERT(tmp);

    tmp = catch(rc = sscanf("foo", "%((?z)foo)", a));
    ASSERT(tmp);
  };

  set_config(321, old);
  if (err) {
    error(err);
  }
}
