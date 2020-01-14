// Test JSON sefun

void do_tests() {
  if(find_object("/std/json")) {
    // TODO: add more test cases

    ASSERT_EQ(0, json_decode("0"));
    ASSERT_EQ(0.0, json_decode("0e0"));
    ASSERT_EQ(0.0, json_decode("0E12328"));
    ASSERT_EQ(to_float(0), json_decode("0.000"));

    // Decoding
    ASSERT_EQ(({0,1,2,3,4,5,0,4,0.0}), json_decode("[0,1,2,3,4,5,-0,4,0.0]"));
    ASSERT_EQ(({}), json_decode("[]"));

    ASSERT_EQ("你好", json_decode("\"\\u4f60\\u597d\""));
    ASSERT_EQ("😄", json_decode("\"\\ud83d\\ude04\""));

    // Encoding
    ASSERT_EQ("[1,2,3,4]", json_encode(({1,2,3,4})));
  }
}
