int charAt(string s, int i) {
  return s[i];
}

int charAtRes(string s, int i) {
  return s[<i];
}

void do_tests() {
  string tmp = "abcdefg";
  string oob = "*String index out of bounds.\n";
  string roob = "*String rindex out of bounds.\n";
  string oob_l = "*Index out of bounds in string index lvalue.\n";

  // INDEX
  ASSERT_EQ('a', tmp[0]);
  ASSERT_EQ('b', tmp[1]);
  ASSERT_EQ('c', tmp[2]);
  ASSERT_EQ('g', tmp[strlen(tmp) - 1]);
  ASSERT_EQ('f', tmp[strlen(tmp) - 2]);

  // INDEX at strlen() return 0
  ASSERT_EQ(0, catch(charAt(tmp, strlen(tmp))));

  // OOB cases
  ASSERT_EQ(oob, catch(charAt(tmp, -1 * strlen(tmp))));
  ASSERT_EQ(oob, catch(charAt(tmp, 999999)));
  ASSERT_EQ(oob, catch(charAt(tmp, -999999)));

  // INDEX on empty string
  ASSERT_EQ(0, catch(charAt("", 0)));
  ASSERT_EQ(oob, catch(charAt("", 999)));
  ASSERT_EQ(oob, catch(charAt("", -1)));
  ASSERT_EQ(oob, catch(charAt("", -999)));
  ASSERT_EQ(oob, catch(charAt("", -999)));

  // INDEX_LVALUE
  tmp[3] = '#';
  ASSERT_EQ("abc#efg", tmp);
  tmp[3] = 'd';
  ASSERT_EQ(oob_l, catch(tmp[strlen(tmp)] = 'a'));
  ASSERT_EQ(oob_l, catch(tmp[999] = 'a'));
  ASSERT_EQ(oob_l, catch(tmp[-1] = 'a'));
  ASSERT_EQ(oob_l, catch(tmp[-999] = 'a'));

  // RINDEX
  ASSERT_EQ('g', tmp[<1]);
  ASSERT_EQ('f', tmp[<2]);
  ASSERT_EQ('a', charAtRes(tmp, strlen(tmp)));
  ASSERT_EQ('b', charAtRes(tmp, strlen(tmp) - 1));

  // RINDEX at 0 returns 0
  ASSERT_EQ(0, catch(charAtRes(tmp, 0)));
  ASSERT_EQ(0, catch(charAtRes("", 0)));

  // Out of bounds
  ASSERT_EQ(roob, catch(charAtRes(tmp, strlen(tmp) + 1)));
  ASSERT_EQ(roob, catch(charAtRes(tmp, strlen(tmp) + 1)));

  // RINDEX on empty string
  ASSERT_EQ(0, catch(charAtRes("", 0)));
  ASSERT_EQ(roob, catch(charAtRes("", 999)));
  ASSERT_EQ(roob, catch(charAtRes("", -1)));
  ASSERT_EQ(roob, catch(charAtRes("", -999)));
  ASSERT_EQ(roob, catch(charAtRes("", -999)));

  // RINDEX_LVALUE
  tmp[<4] = '#';
  ASSERT_EQ("abc#efg", tmp);
  tmp[<4] = 'd';

  ASSERT_EQ(oob_l, catch(tmp[<0] = 'a'));
  ASSERT_EQ(oob_l, catch(tmp[<strlen(tmp) + 1] = 'a'));
  ASSERT_EQ(oob_l, catch(tmp[<999] = 'a'));
  ASSERT_EQ(oob_l, catch(tmp[<-1] = 'a'));
  ASSERT_EQ(oob_l, catch(tmp[<-999] = 'a'));

  // RANGE
  ASSERT_EQ("a", tmp[0..0]);
  ASSERT_EQ(tmp, tmp[0..strlen(tmp) - 1]);
  ASSERT_EQ("abc", tmp[0..2]);
  ASSERT_EQ("def", tmp[3..5]);
  ASSERT_EQ("bcdefg", tmp[<6..6]);

  // lvalue out of bound
  ASSERT(catch(tmp[0..strlen(tmp)] = ""));
  ASSERT(catch(tmp[0..999] = ""));
  ASSERT(catch(tmp[strlen(tmp)..strlen(tmp)]  = ""));
  ASSERT(catch(tmp[999..0] = ""));

  ASSERT(catch(tmp[<0..<1] = ""));
  ASSERT(catch(tmp[<999..999] = ""));
  ASSERT(catch(tmp[<strlen(tmp)..strlen(tmp)] = ""));
}
