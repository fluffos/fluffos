/**
 * Test suite for json_parse() and json_stringify() native efuns
 *
 * Tests the native JSON package implementation.
 */

void do_tests() {
  string content = "";

  // Basic types - json_parse
  ASSERT_EQ(0, json_parse("0"));
  ASSERT_EQ("test", json_parse("\"test\""));
  ASSERT_EQ(1, json_parse("true"));
  ASSERT_EQ(0, json_parse("false"));
  ASSERT_EQ(0, json_parse("null"));

  // Arrays - json_parse
  ASSERT_EQ(({1,2,3}), json_parse("[1,2,3]"));
  ASSERT_EQ(({}), json_parse("[]"));
  ASSERT_EQ(({1, "two", 3}), json_parse("[1,\"two\",3]"));

  // Objects (mappings) - json_parse
  mixed obj = json_parse("{\"key\": \"value\"}");
  ASSERT_EQ("value", obj["key"]);

  // Nested structures - json_parse
  mixed nested = json_parse("{\"items\": [1, 2, 3], \"meta\": {\"count\": 3}}");
  ASSERT_EQ(3, sizeof(nested["items"]));
  ASSERT_EQ(3, nested["meta"]["count"]);

  // Unicode handling - json_parse
  ASSERT_EQ("你好", json_parse("\"你好\""));

  // Stringify tests
  ASSERT_EQ("42", json_stringify(42));
  ASSERT_EQ("\"test\"", json_stringify("test"));
  ASSERT_EQ("[1,2,3]", json_stringify(({1,2,3})));

  // Roundtrip with complex structure
  mixed original = ([ "items": ({1,2,3}), "name": "test" ]);
  string json_str = json_stringify(original);
  mixed restored = json_parse(json_str);
  ASSERT_EQ(3, sizeof(restored["items"]));
  ASSERT_EQ("test", restored["name"]);

  // Pretty printing (with indent parameter)
  string pretty = json_stringify(({1, 2, 3}), 2);
  ASSERT(strsrch(pretty, "\n") != -1, "Pretty print has newlines");

  // Test with actual file content
  content = read_file("/single/tests/std/test.json");
  if(content) {
    ASSERT(json_stringify(json_parse(content)));
  }
}
