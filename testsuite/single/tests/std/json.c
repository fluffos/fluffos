// Test JSON sefun

void do_tests() {
  string content = "";

  trace_start("trace_json.json");

  if(!find_object("/std/json")) {
    write("json not loaded, skipped");
    return;
  }
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

  content = read_file("/single/tests/std/test.json");
  ASSERT(content);
  ASSERT(json_encode(json_decode(content)));

  // Handle \e
  content = "\e你好";
  ASSERT_EQ(content, json_decode(json_encode(content)));

  // Handle \x in LPC
  content = "\x1b你好";
  ASSERT_EQ(content, json_decode(json_encode(content)));

  // e2e
  content = read_file("/single/tests/std/test2.json");
  ASSERT(content);
  ASSERT(json_encode(json_decode(content)));

  trace_end();
}
