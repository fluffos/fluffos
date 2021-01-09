void do_tests() {
  int x = 100;
  string* tmp1 = ({ "x", "y", "z" });
  mapping tmp = ([ "ab": 10, 20: "ab", tmp1: "abc" ]);
  string str = "(ab)"; // String to run through sscanf()
  string fail = "";

  ASSERT_EQ(10, tmp["ab"]);
  ASSERT_EQ(10, tmp["a" + "b"]);
  ASSERT_EQ(10, tmp[tmp[20]]);

  sscanf(str, "(%s)", fail);
  ASSERT_EQ("ab", fail);
  ASSERT_EQ(10, tmp[fail]);


  ASSERT_EQ("ab", tmp[20]);
  ASSERT_EQ("ab", tmp[10 * 2]);
  ASSERT_EQ("ab", tmp[x/5]);
}
