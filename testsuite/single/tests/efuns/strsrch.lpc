void do_tests() {
  string txt = "Every day is a new day. It is better to be lucky. But I would rather be exact. 不过话得说回来，没有一桩事是容易的。 Then when luck comes you are ready. ";

  // single code point search
  ASSERT_EQ(5, strsrch(txt, ' '));
  ASSERT_EQ(46, strsrch(txt, 'k'));
  ASSERT_EQ(79, strsrch(txt, "不过"[0]));
  ASSERT_EQ(80, strsrch(txt, "不过"[1]));

  // special case
  ASSERT_EQ(134, strsrch(txt, '\0'));

  // NOT FOUND
  ASSERT_EQ(-1, strsrch(txt, 'z'));
  ASSERT_EQ(-1, strsrch(txt, "我"[0]));

  // short ascii string
  ASSERT_EQ(43, strsrch(txt, "luck"));
  ASSERT_EQ(113, strsrch(txt, "come"));
  ASSERT_EQ(100, strsrch(txt, "en w"));
  // long ascii string
  ASSERT_EQ(50, strsrch(txt, "But I would"));
  ASSERT_EQ(50, strsrch(txt, "But I would"));

  // non ascii string
  ASSERT_EQ(86, strsrch(txt, "，没有一"));

  txt = @TEXT
HTTP/1.1 200 OK
Date: Fri, 16 Jul 2021 05:31:40 GMT
Server: Application/debug ktor/debug
Content-Length: 31
Content-Type: application/json; charset=UTF-8

{"code":0,"session":"E4EtZGu4"}
TEXT;
  ASSERT_EQ(155, strsrch(txt, "{"));
}
