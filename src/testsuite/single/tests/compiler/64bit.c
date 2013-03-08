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
  return ({ "double plus float", 0,
      (sprintf("%.16f", 1 + 1.0000000000000002) ==
            "2.0000000000000002")
  });
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
