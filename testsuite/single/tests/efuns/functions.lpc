inherit "/inherit/tests";

int func() {
  return 1;
}

int func1(string moo, mixed args...) {
  func();
  return 1;
}

void do_tests()
{
  // FIXME: in compiler.cc, the function table is sorted using pointer address, so the return result here
  // is not deterministic
  ASSERT_EQ(
    (({ /* sizeof() == 5 */
      "describe_test",
      "do_tests",
      "func",
      "func1",
      "query_test_info",
    })), sort_array(functions(this_object(), 0), 0));
  ASSERT_EQ(
    ({ /* sizeof() == 5 */
      ({ /* sizeof() == 4 */
        "describe_test",
        1,
        "void",
        "string"
      }),
      ({ /* sizeof() == 3 */
        "do_tests",
        0,
        "void"
      }),
      ({ /* sizeof() == 3 */
        "func",
        0,
        "int"
      }),
      ({ /* sizeof() == 5 */
        "func1",
        2,
        "int",
        "string",
        "mixed"
      }),
      ({ /* sizeof() == 3 */
         "query_test_info",
         0,
         "string"
      }),
    }), sort_array(functions(this_object(), 1), (: strcmp($1[0], $2[0]) :)));
  ASSERT_EQ(
    ({ /* sizeof() == 3 */
      "do_tests",
      "func",
      "func1",
    }), sort_array(functions(this_object(), 2), 0));
  ASSERT_EQ(
    ({ /* sizeof() == 3 */
      ({ /* sizeof() == 3 */
        "do_tests",
        0,
        "void"
      }),
      ({ /* sizeof() == 3 */
        "func",
        0,
        "int"
      }),
      ({ /* sizeof() == 5 */
        "func1",
        2,
        "int",
        "string",
        "mixed"
      }),
    }), sort_array(functions(this_object(), 3), (: strcmp($1[0], $2[0]) :)));
}
