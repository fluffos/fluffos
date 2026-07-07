// This file should be dos-style, kept at CRLF, see .gitattributes in this directory
void do_tests() {
  string *str;
  string text = @TEXT
Two roads diverged in a yellow wood,
And sorry I could not travel both.
TEXT;
  str = @@ARRAY
Two roads diverged in a yellow wood,
And sorry I could not travel both.
ARRAY;

  ASSERT_EQ(text, "Two roads diverged in a yellow wood,\nAnd sorry I could not travel both.\n");
  ASSERT_EQ(sizeof(str), 2);
  ASSERT_EQ(str[0], "Two roads diverged in a yellow wood,");
  ASSERT_EQ(str[1], "And sorry I could not travel both.");
}
