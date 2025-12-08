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
  string tmp, re;

#ifndef __PACKAGE_PCRE__
  write("PACKAGE_PCRE not defined, test skipped.");
  return ;
#endif

  ASSERT_EQ(1, pcre_match("123", "^[0-9]+$")); // or other pattern
  ASSERT(same_array(
    pcre_assoc("testhahatest", ({ "haha", "te" }), ({ 2, 3 }), 4),
    ({ ({ "", "te", "st", "haha", "", "te", "st" }),
        ({ 4, 3, 4, 2, 4, 3, 4 }) })));
  ASSERT(catch(pcre_assoc("foo", ({ 1 }), ({ 2, 3 }))));
  ASSERT(catch(pcre_assoc("foo", ({ 1, 2 }), ({ 2, 3 }))));
  ASSERT(catch(pcre_assoc("foo", ({ "bar", "+" }), ({ 0, 1 }))));

  ASSERT_EQ(({"bb"}), pcre_extract( "bbracadabra", "([abc]+)" ));

  // https://tools.ietf.org/html/rfc3986#appendix-B
  ASSERT_EQ(
      ({ /* sizeof() == 1 */
        ({ /* sizeof() == 10 */
          "http://www.ics.uci.edu/pub/ietf/uri/#Related",
              "http:",
              "http",
              "//www.ics.uci.edu",
              "www.ics.uci.edu",
              "/pub/ietf/uri/",
              "",
              "",
              "#Related",
              "Related"
        })
      }),
      pcre_match_all("http://www.ics.uci.edu/pub/ietf/uri/#Related",
                           "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?" ));

  // multiline phone number
  tmp = @TEXT
984-662-2681
747-251-7561
052-247-6028
747-405-0262
146-361-2917
775-203-2073
TEXT;
  re = "(?m)^(?:(?:\\+?1\\s*(?:[.-]\\s*)?)?(?:\\(\\s*([2-9]1[02-9]|[2-9][02-8]1|[2-9][02-8][02-9])\\s*\\)|([2-9]1[02-9]|[2-9][02-8]1|[2-9][02-8][02-9]))\\s*(?:[.-]\\s*)?)?([2-9]1[02-9]|[2-9][02-9]1|[2-9][02-9]{2})\\s*(?:[.-]\\s*)?([0-9]{4})(?:\\s*(?:#|x\\.?|ext\\.?|extension)\\s*(\\d+))?$";
  ASSERT_EQ(
      ({ /* sizeof() == 4 */
        ({ /* sizeof() == 5 */
          "984-662-2681",
              "",
              "984",
              "662",
              "2681"
        }),
            ({ /* sizeof() == 5 */
              "747-251-7561",
                  "",
                  "747",
                  "251",
                  "7561"
            }),
            ({ /* sizeof() == 5 */
              "747-405-0262",
                  "",
                  "747",
                  "405",
                  "0262"
            }),
            ({ /* sizeof() == 5 */
              "775-203-2073",
                  "",
                  "775",
                  "203",
                  "2073"
            })
      }),
      pcre_match_all(tmp, re));

  tmp = "However, if you need to match after every line terminator";
  re = "(?:[d-z])([a-z]([a-z]+))";
  ASSERT_EQ(
      ({ /* sizeof() == 8 */
        ({ /* sizeof() == 3 */
          "owever",
              "wever",
              "ever"
        }),
            ({ /* sizeof() == 3 */
              "you",
                  "ou",
                  "u"
            }),
            ({ /* sizeof() == 3 */
              "need",
                  "eed",
                  "ed"
            }),
            ({ /* sizeof() == 3 */
              "match",
                  "atch",
                  "tch"
            }),
            ({ /* sizeof() == 3 */
              "fter",
                  "ter",
                  "er"
            }),
            ({ /* sizeof() == 3 */
              "every",
                  "very",
                  "ery"
            }),
            ({ /* sizeof() == 3 */
              "line",
                  "ine",
                  "ne"
            }),
            ({ /* sizeof() == 3 */
              "terminator",
                  "erminator",
                  "rminator"
            })
      }),
      pcre_match_all(tmp, re));

  tmp = "alpha 99";
  re = "(?<word>\\w+) (?<num>\\d+)";
  mixed *named = pcre_extract(tmp, re, 1);
  ASSERT_EQ(3, sizeof(named));
  ASSERT_EQ("alpha", named[0]);
  ASSERT_EQ("99", named[1]);
  ASSERT_EQ((["word": "alpha", "num": "99"]), named[2]);

  ASSERT_EQ(({"alpha", "99"}), pcre_extract(tmp, re));

  // No named groups but include flag => empty mapping appended.
  mixed *with_empty_map = pcre_extract("abc", "(a)(b)(c)", 1);
  ASSERT_EQ(4, sizeof(with_empty_map));
  ASSERT_EQ(([ ]), with_empty_map[3]); // empty mapping
  ASSERT_EQ("a", with_empty_map[0]);

  // Named group present but not participating should yield empty mapping entry.
  mixed *optional_named = pcre_extract("ac", "(a)(?<b>b)?(c)", 1);
  ASSERT_EQ(4, sizeof(optional_named));
  ASSERT_EQ("a", optional_named[0]);
  ASSERT_EQ("", optional_named[1]);
  ASSERT_EQ("c", optional_named[2]);
  ASSERT_EQ(([ ]), optional_named[3]);

  int flag_i = 1 << 16; // PCRE_I
  ASSERT_EQ("xbc", pcre_replace("abc", "(A)", ({"x"}), flag_i));
  ASSERT_EQ("a!bc", pcre_replace_callback("abc", "(A)", (: $1 + "!" :), flag_i));

  // pcre_match with flags: case-insensitive and anchored.
  ASSERT_EQ(1, pcre_match("abc", "ABC", flag_i));
  int flag_anchored = (1 << 21); // PCRE_A
  ASSERT_EQ(0, pcre_match("zabc", "abc", flag_anchored));

  // pcre_match_all with flags: multiline
  int flag_m = (1 << 17); // PCRE_M
  mixed *ma = pcre_match_all("a\nb\nc", "^b", flag_m);
  ASSERT_EQ(1, sizeof(ma));
  ASSERT_EQ("b", ma[0][0]);

  // pcre_extract with flags (case-insensitive)
  mixed *ex = pcre_extract("HELLO", "(hello)", 0, flag_i);
  ASSERT_EQ(({"HELLO"}), ex);

  // pcre_assoc with flags: case-insensitive match
  mixed *assoc_res = pcre_assoc("HelloWorld", ({"hello"}), ({1}), 0, flag_i);
  ASSERT_EQ(({({"", "Hello", "World"}), ({0,1,0})}), assoc_res);

  tmp = "foobar";
  ASSERT_EQ("foo->bar", pcre_replace_callback(tmp, "(foo)", (: $1 + "->" :)));
}
