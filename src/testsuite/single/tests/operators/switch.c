int switch1(string x) {
  switch(x) {
    case "a":
      return 1;
    case "obj":
      return 1;
    default:
      return 999;
    case "cof":
    case "dsu":
      return 3;
    case "b":
      return 2;
    case 0:
      return 10;
  }
}

int switch2(int x) {
  switch(x) {
    case 0:
      return 0;
    case 1:
      return 1;
    case -1:
      return 7;
    case 30:
    case 40:
      return 3;
    case 10:
      return 2;
    default:
      return 999;
    case 0x7ffffffe:
      return 5;
    case 0x7fffffffffffffee:
      return 4;
    case -123456:
      return 6;
    case MAX_INT:
      return 8;
    case MAX_INT - 10:
      return 9;
  }
}

int switch3(int x) {
  switch(x) {
    case 0..0:
      return 0;
    case 11..20:
      return 2;
    case 1..10:
      return 1;
    case 21..:
      return 999;
  }
}

int switch4(int x) {
  switch(x) {
    case ..20:
      return 1;
    default:
      return 999;
  }
}

/* direct lookup switch.*/
int switch5(int x) {
  switch(x) {
    case 1:
      return 1;
    case 2:
      return 2;
  }
  return 999;
}

void do_tests() {
  // string tests
  ASSERT_EQ(1, switch1("a"));
  ASSERT_EQ(1, switch1("obj"));
  ASSERT_EQ(2, switch1("b"));
  ASSERT_EQ(3, switch1("cof"));
  ASSERT_EQ(3, switch1("dsu"));
  ASSERT_EQ(999, switch1("aasdfk"));
  ASSERT_EQ(999, switch1("bcd"));
  ASSERT_EQ(999, switch1("whatever"));
  ASSERT_EQ(10, switch1(0));

  // int tests
  ASSERT_EQ(0, switch2(0));
  ASSERT_EQ(1, switch2(1));
  ASSERT_EQ(2, switch2(10));
  ASSERT_EQ(3, switch2(30));
  ASSERT_EQ(3, switch2(40));
  ASSERT_EQ(4, switch2(0x7fffffffffffffee));
  ASSERT_EQ(5, switch2(0x7ffffffe));
  ASSERT_EQ(6, switch2(-123456));
  ASSERT_EQ(7, switch2(-1));
  ASSERT_EQ(8, switch2(MAX_INT));
  ASSERT_EQ(9, switch2(MAX_INT - 10));
  ASSERT_EQ(999, switch2(123892));

  // range tests
  ASSERT_EQ(0, switch3(0));
  for(int i=1; i<=10; i++) {
    ASSERT_EQ(1, switch3(i));
  }
  for(int i=11; i<=20; i++) {
    ASSERT_EQ(2, switch3(i));
  }
  for(int i=21; i<=100; i++) {
    ASSERT_EQ(999, switch3(i));
  }
  ASSERT_EQ(999, switch3(MAX_INT));

  // range test 2
  for(int i=-20; i<=20; i++) {
    ASSERT_EQ(1, switch4(i));
  }
  for(int i=21; i<=100; i++) {
    ASSERT_EQ(999, switch4(i));
  }
  ASSERT_EQ(999, switch4(MAX_INT));

  // direct lookup table
  ASSERT_EQ(1, switch5(1));
  ASSERT_EQ(2, switch5(2));
  ASSERT_EQ(999, switch5(-1));
  ASSERT_EQ(999, switch5(1234));
}
