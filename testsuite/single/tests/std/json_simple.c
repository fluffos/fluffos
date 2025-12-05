// Simplified JSON tests

void do_tests() {
  string encoded;
  mixed decoded;
  mapping m;

  if(!find_object("/std/json")) {
    write("json not loaded, skipped\n");
    return;
  }

  write("=== Testing JSON Implementation ===\n");

  // Basic decode tests
  ASSERT_EQ(0, json_decode("0"));
  ASSERT_EQ(1, json_decode("1"));
  ASSERT_EQ(-1, json_decode("-1"));
  ASSERT_EQ(42, json_decode("42"));

  // Float tests
  ASSERT_EQ(0.0, json_decode("0.0"));
  ASSERT_EQ(1.5, json_decode("1.5"));
  ASSERT_EQ(-3.14, json_decode("-3.14"));

  // String tests
  ASSERT_EQ("", json_decode("\"\""));
  ASSERT_EQ("hello", json_decode("\"hello\""));
  ASSERT_EQ("\"", json_decode("\"\\\"\""));
  ASSERT_EQ("\\", json_decode("\"\\\\\""));
  ASSERT_EQ("\n", json_decode("\"\\n\""));
  ASSERT_EQ("\t", json_decode("\"\\t\""));

  // Unicode tests
  ASSERT_EQ("你好", json_decode("\"\\u4f60\\u597d\""));
  ASSERT_EQ("😄", json_decode("\"\\ud83d\\ude04\""));

  // Boolean/null tests
  ASSERT_EQ(1, json_decode("true"));
  ASSERT_EQ(0, json_decode("false"));
  ASSERT_EQ(0, json_decode("null"));

  // Array tests
  ASSERT_EQ(({}), json_decode("[]"));
  ASSERT_EQ(({1}), json_decode("[1]"));
  ASSERT_EQ(({1,2,3}), json_decode("[1,2,3]"));
  ASSERT_EQ(({1,"hello",1,0}), json_decode("[1,\"hello\",true,false]"));
  ASSERT_EQ(({({1,2}),({3,4})}), json_decode("[[1,2],[3,4]]"));

  // Basic encoding tests
  ASSERT_EQ("0", json_encode(0));
  ASSERT_EQ("1", json_encode(1));
  ASSERT_EQ("-1", json_encode(-1));
  ASSERT_EQ("\"hello\"", json_encode("hello"));
  ASSERT_EQ("[]", json_encode(({})));
  ASSERT_EQ("[1,2,3]", json_encode(({1,2,3})));

  // Round-trip tests
  encoded = json_encode(42);
  decoded = json_decode(encoded);
  ASSERT_EQ(42, decoded);

  encoded = json_encode("hello world");
  decoded = json_decode(encoded);
  ASSERT_EQ("hello world", decoded);

  encoded = json_encode(({1,2,3,4,5}));
  decoded = json_decode(encoded);
  ASSERT_EQ(({1,2,3,4,5}), decoded);

  // Special character round-trip
  encoded = json_encode("\e你好");
  decoded = json_decode(encoded);
  ASSERT_EQ("\e你好", decoded);

  encoded = json_encode("\x1b你好");
  decoded = json_decode(encoded);
  ASSERT_EQ("\x1b你好", decoded);

  // Test real JSON files
  encoded = read_file("/single/tests/std/test.json");
  ASSERT(encoded);
  decoded = json_decode(encoded);
  ASSERT(decoded);
  ASSERT(json_encode(decoded));

  encoded = read_file("/single/tests/std/test2.json");
  ASSERT(encoded);
  decoded = json_decode(encoded);
  ASSERT(decoded);
  ASSERT(json_encode(decoded));

  write("=== All JSON tests passed! ===\n");
}
