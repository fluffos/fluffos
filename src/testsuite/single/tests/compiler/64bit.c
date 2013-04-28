// These tests can only performed if the driver is compiled for a LP64
// platform. Tests borrowed from LDMUD.
// https://raw.github.com/ldmud/ldmud/master/test/t-LP64-issues.c
#if MAX_INT > 2147483647
mixed* func1() {
  return ({ "0000770-1 (bitwise AND)", 0,
    ((0x1000000000 & 0x1000000000) == 0x1000000000)
  });
}

mixed* func2() {
  int i = 0x1000000000;
  i &= 0x1000000000;

  return ({ "0000770-2 (bitwise AND assignment)", 0,
      (i == 0x1000000000)
  });
}

mixed* func3() {
  return ({ "0000770-3 (bitwise OR)", 0,
      ((0x1000000000 | 0x0) == 0x1000000000)
  });
}

mixed* func4() {
  int i = 0x1000000000;
  i |= 0x0;

  return ({ "0000770-4 (bitwise OR assignment)", 0,
      (i == 0x1000000000)
  });
}

mixed* func5() {
  return ({ "0000770-5 (Modulus)", 0,
      ((216681099904 % 100000000000) == 16681099904)
  });
}

mixed* func6() {
  int i = 216681099904;
  i %= 100000000000;

  return ({ "0000770-6 (Modulus assignment)", 0,
      (i == 16681099904)
  });
}

mixed* func7() {
  return ({ "0000770-7 (bitwise XOR)", 0,
      ((0x10000000000 ^ 0x01000000000) == 0x11000000000)
  });
}

mixed* func8() {
  int i = 0x10000000000;
  i ^= 0x01000000000;
  return ({ "0000770-8 (bitwise XOR assignment)", 0,
      (i == 0x11000000000)
  });
}
#endif

#if defined(MAX_FLOAT)
mixed* func_f1() {
  return ({ "float is C double:", 0,
      sprintf("%.16f",
        (1.0 / 3.0)) ==
        "0.3333333333333333"
  });
}

mixed* func_f2() {
  float i = 1.0;
  i /= 3.0;
  return ({ "float is C double", 0,
      (sprintf("%.16f", i) ==
            "0.3333333333333333")
  });
}

mixed* func_f3() {
  return ({ "float literal is C double", 0,
      (sprintf("%.16f", 1.0000000000000002) ==
            "1.0000000000000002")
  });
}

mixed* func_f4() {
  float i = 1.0000000000000002;
  return ({ "float literal is C double", 0,
      (sprintf("%.16f", i) ==
           "1.0000000000000002")
  });
}

mixed* func_f5() {
  // "int plus float"
  ASSERT_EQ(2.00000000000002,
            1 + 1.00000000000002);
}

mixed* func_f6() {
  // "float plus float"
  ASSERT_EQ(4.00000000000000,
            3.00000000000000 +
            1.00000000000000);
}

mixed* func_constant_overflow1() {
  // "constant int overflow", 0,
  ASSERT_EQ(0x7fffffffffffffff, MAX_INT);
  // Bug: When mudos parse LPC file, it treats '-123'
  // as '-1 * 123'. when reading MIN_INT '-9223372036854775808'
  // it will be parsed as -1 * 9223372036854775808.
  // and 9223372036854775808 will cause overflow in strtoll(),
  // which will return 9223372036854775807 (INT_MAX) instead.
  // So MIN_INT == -1 * MAX_INT.
  ASSERT_EQ(-0x7fffffffffffffff, -MAX_INT);
  ASSERT_EQ("-9223372036854775807", sprintf("%d", MIN_INT));
  ASSERT(MIN_INT != MAX_INT + 1);
  ASSERT(MIN_INT == -1 * MAX_INT);
  ASSERT_EQ("-9223372036854775808", sprintf("%d", MAX_INT + 1));
  ASSERT_EQ("-9223372036854775807", sprintf("%d", -0x7fffffffffffffff));

  // FluffOS use strtoll to parse constant,
  // which always returns MAX_INT / MIN_INT when that happens.
  ASSERT_EQ(-0x7fffffffffffffff1234, MIN_INT);
  ASSERT_EQ(0x7fffffffffffffff1234, MAX_INT);

  ASSERT_EQ(8142508126285861427, 0x70ffffffffffffff + 0x1234);
  ASSERT_EQ(9223372036854771149, -0x7fffffffffffffff - 0x1234);

  ASSERT_EQ(sprintf("%d", MAX_INT), "" + MAX_INT);
  ASSERT_EQ(sprintf("%d", MIN_INT), "" + MIN_INT);
}

mixed* func_constant_overflow2() {
  float max_float = 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000;

  ASSERT_EQ(max_float, MAX_FLOAT);
  ASSERT_EQ(-max_float, -MAX_FLOAT);
  ASSERT_EQ(sprintf("%f", MAX_FLOAT), "" + MAX_FLOAT);
  ASSERT_EQ(sprintf("%f", MIN_FLOAT), "" + MIN_FLOAT);
}

mixed* func_sscanf() {
  int s = 0;
  string res = "179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000";
  float f;

  sscanf("9223372036854775807", "%d", s);
  ASSERT_EQ(MAX_INT, s);
  sscanf("1000009223372036854775807", "%d", s);
  ASSERT_EQ(MAX_INT, s);

  sscanf("-9223372036854775807", "%d", s);
  ASSERT_EQ(-1 * MAX_INT, s);

  sscanf("-9223372036854775807", "%d", s);
  ASSERT_EQ(-1 * MAX_INT, s);

  sscanf(res, "%f", f);
  ASSERT_EQ(MAX_FLOAT, f);
>>>>>>> f9cdbcf... Include #ifndef routine in auto generated header.
}
#endif

nosave mixed *tests = ({
#if MAX_INT > 2147483647
  (: func1 :),
  (: func2 :),
  (: func3 :),
  (: func4 :),
  (: func5 :),
  (: func6 :),
  (: func7 :),
  (: func8 :),
#endif
#if defined(MAX_FLOAT)
  (: func_f1 :),
  (: func_f2 :),
  (: func_f3 :),
  (: func_f4 :),
#endif
});

void do_tests() {
#if MAX_INT <= 2147483647
  write("LPC int is not 64 bit, test not run: MAX_INT:" + MAX_INT + "\n");
#else
  foreach(mixed test in tests) {
    mixed* result = evaluate(test);
    ASSERT2(result[2], result[0]);
  }
#endif
}
