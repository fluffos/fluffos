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
}
