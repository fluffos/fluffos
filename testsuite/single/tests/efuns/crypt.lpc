void test_oldcrypt(string key) {
  string result;

  ASSERT_EQ("mckfcn`d!glafl`lnam`hikdfaeja`eccckjeinjm", oldcrypt("123", "123"));

  result = oldcrypt(key, "");
  ASSERT_EQ(result, oldcrypt(key, result));
}

void test_crypt(string key) {
  string result;

  // Old algo/salt is DES with 2 character key and maximum pasword length is 8
  ASSERT_EQ("12IbR.gJ8wcpc", crypt("123", "123"));
  ASSERT_EQ("12IbR.gJ8wcpc", crypt("123", "12IbR.gJ8wcpc"));

  // yes, it is very broken
  ASSERT_EQ("11abPRGlgbz9A", crypt("deadbeef", "11"));
  ASSERT_EQ("11abPRGlgbz9A", crypt("deadbeef-wtf", "11-wtf"));

  // Make sure we use SHA512 as default
  result = crypt(key, "");
  ASSERT_EQ("$6$", result[0..2]);
  ASSERT_EQ(result, crypt(key, result));

  // Make sure we support $1$, $2a$, $2x$, $2y$, $5$, $6$ etc
  result = crypt(key, "$1$12IbRgJ8$92eJ54pYS5ISsRs./Jac21");
  ASSERT_EQ("$1$12IbRgJ8$92eJ54pYS5ISsRs./Jac21", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$2a$05$abcdefghijklmnopqrstuu");
  ASSERT_EQ("$2a$05$abcdefghijklmnopqrstuunlwTxQ4sl7dLGS2xJ.iMMzU7tVomKuy", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$2x$08$abcdefghijklmnopqrstuu");
  ASSERT_EQ("$2x$08$abcdefghijklmnopqrstuuwGS8NeHQOi8O1YXuHuWBzYTYdR1l8GS", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$2y$09$abcdefghijklmnopqrstuu");
  ASSERT_EQ("$2y$09$abcdefghijklmnopqrstuux4Xxxa5jKMzA60uXAXbkP/vAphqHo.q", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$5$xxpyPxiuc0IJY");
  ASSERT_EQ("$5$xxpyPxiuc0IJY$BAJWByjsbjtPgIWKrBdSZes5VdZWyvYr2CZ3pmOzRr4", result);
  ASSERT_EQ(result, crypt(key, result));

  result = crypt(key, "$6$DSVWtVeRU/71A");
  ASSERT_EQ("$6$DSVWtVeRU/71A$GvRozPOkmWEd3wTR7wMMQhug5wM5rrEM/cle4VEZD1yDEGf1iq6JST0PDN1Ua6QaQslVK2jAzIBKOamxi5dt1.", result);
  ASSERT_EQ(result, crypt(key, result));

  // $2$ on the other hand, is wrong format
  result = crypt(key, "$2$zsCyqpCmwWE2g");
  ASSERT(result[0..2] != "$2");
  ASSERT_EQ(result, crypt(key, result));
}

void do_tests() {
  string key, result;
  // Chosen by fair dice roll. guaranteed to be random.
  key = "U?5测mG$'xe`%!pf6试p\"#";

  test_crypt(key);
  test_oldcrypt(key);

  // Make sure crypt/oldcrypt mix usage don't fail
  result = crypt(key, "");
  ASSERT(result != oldcrypt(key, result));
  result = oldcrypt(key, "");
  ASSERT(result != crypt(key, result));

  // Make sure invalid second parameter doesn't crash
  ASSERT(crypt(key, key));
  ASSERT(oldcrypt(key, key));

}
