void test_oldcrypt(string key) {
  string result;

  ASSERT_EQ("mckfcn`d!glafl`lnam`hikdfaeja`eccckjeinjm", oldcrypt("123", "123"));

  result = oldcrypt(key, "");
  ASSERT_EQ(result, oldcrypt(key, result));
}

void test_crypt(string key) {
  string result;

  // Default algo/salt is DES
  ASSERT_EQ("12IbR.gJ8wcpc", crypt("123", "123"));
  ASSERT_EQ("12IbR.gJ8wcpc", crypt("123", "12IbR.gJ8wcpc"));

  result = crypt(key, "");
  ASSERT_EQ(result, crypt(key, result));

  // Make sure we support $1$, $2a$, $2x$, $2y$, $5$, $6$ etc
  result = crypt(key, "$1$12IbRgJ8$Y.vPnUxb7UxG.2bnEVAny.");
  ASSERT_EQ("$1$12IbRgJ8$Y.vPnUxb7UxG.2bnEVAny.", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$2a$05$abcdefghijklmnopqrstuu");
  ASSERT_EQ("$2a$05$abcdefghijklmnopqrstuuUUDe2kD62zFEBivC.bhb00frGEXsinq", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$2x$08$abcdefghijklmnopqrstuu");
  ASSERT_EQ("$2x$08$abcdefghijklmnopqrstuuQa7IQF7.7DwNJ1gNo9ChNEpDaymSfYq", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$2y$09$abcdefghijklmnopqrstuu");
  ASSERT_EQ("$2y$09$abcdefghijklmnopqrstuuMwuBdFTxWJl8CdA0t6CpIt/rSERMOb.", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$5$xxpyPxiuc0IJY");
  ASSERT_EQ("$5$xxpyPxiuc0IJY$OYOh92l2UEyk/Rb.cYlcgbbZ5zsP3TVErs4Jyvovmg5", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$6$DSVWtVeRU/71A");
  ASSERT_EQ("$6$DSVWtVeRU/71A$Z8TYP8XyUv.Qehba.tcxgcBT7XzON2rPBVYbGC3sCxvO8nu7KNRbq3GUoHH.yyJGSMmvWmTsNwHBCA1fHVGKI1", result);
  ASSERT_EQ(result, crypt(key, result));

  // $2$ on the other hand, is wrong format
  result = crypt(key, "$2$zsCyqpCmwWE2g");
  ASSERT(result[0..2] != "$2");
  ASSERT_EQ(result, crypt(key, result));
}

void do_tests() {
  string key, result;
  // Chosen by fair dice roll. guaranteed to be random.
  key = "U?5mG$'xe`%!pf6p\"";

  test_crypt(key);
  test_oldcrypt(key);

  // Make sure crypt/oldcrypt mix usage don't fail
  result = crypt(key, "");
  ASSERT(result != oldcrypt(key, result));
  result = oldcrypt(key, "");
  ASSERT(result != crypt(key, result));
}
