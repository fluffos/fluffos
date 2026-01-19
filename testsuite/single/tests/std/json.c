// Test JSON sefun

void test_decode_numbers() {
  // Integer zero variants
  ASSERT_EQ(0, json_decode("0"));
  ASSERT_EQ(0, json_decode(" 0 "));
  ASSERT_EQ(0, json_decode("\n0\n"));
  ASSERT_EQ(0, json_decode("\t0\t"));

  // Float zero variants
  ASSERT_EQ(0.0, json_decode("0e0"));
  ASSERT_EQ(0.0, json_decode("0E0"));
  ASSERT_EQ(0.0, json_decode("0E12328"));
  ASSERT_EQ(to_float(0), json_decode("0.000"));
  ASSERT_EQ(0.0, json_decode("0.0"));

  // Positive integers
  ASSERT_EQ(1, json_decode("1"));
  ASSERT_EQ(42, json_decode("42"));
  ASSERT_EQ(12345, json_decode("12345"));
  ASSERT_EQ(2147483647, json_decode("2147483647"));

  // Negative integers
  ASSERT_EQ(-1, json_decode("-1"));
  ASSERT_EQ(-42, json_decode("-42"));
  ASSERT_EQ(-12345, json_decode("-12345"));

  // Positive floats
  ASSERT_EQ(1.5, json_decode("1.5"));
  ASSERT_EQ(3.14159, json_decode("3.14159"));
  ASSERT_EQ(0.123, json_decode("0.123"));

  // Negative floats
  ASSERT_EQ(-1.5, json_decode("-1.5"));
  ASSERT_EQ(-3.14159, json_decode("-3.14159"));
  ASSERT_EQ(-0.123, json_decode("-0.123"));

  // Scientific notation - positive exponent
  ASSERT_EQ(1.0e10, json_decode("1e10"));
  ASSERT_EQ(1.0e10, json_decode("1E10"));
  ASSERT_EQ(1.23e5, json_decode("1.23e5"));
  ASSERT_EQ(1.23e5, json_decode("1.23E5"));
  ASSERT_EQ(1.0e10, json_decode("1e+10"));

  // Scientific notation - negative exponent
  ASSERT_EQ(1.0e-5, json_decode("1e-5"));
  ASSERT_EQ(1.0e-5, json_decode("1E-5"));
  ASSERT_EQ(1.23e-3, json_decode("1.23e-3"));
}

void test_decode_strings() {
  // Basic strings
  ASSERT_EQ("", json_decode("\"\""));
  ASSERT_EQ("hello", json_decode("\"hello\""));
  ASSERT_EQ("hello world", json_decode("\"hello world\""));

  // Escape sequences
  ASSERT_EQ("\"", json_decode("\"\\\"\""));
  ASSERT_EQ("\\", json_decode("\"\\\\\""));
  ASSERT_EQ("/", json_decode("\"\\/\""));
  ASSERT_EQ("\b", json_decode("\"\\b\""));
  ASSERT_EQ("\x0c", json_decode("\"\\f\""));
  ASSERT_EQ("\n", json_decode("\"\\n\""));
  ASSERT_EQ("\r", json_decode("\"\\r\""));
  ASSERT_EQ("\t", json_decode("\"\\t\""));

  // Multiple escapes
  ASSERT_EQ("line1\nline2\nline3", json_decode("\"line1\\nline2\\nline3\""));
  ASSERT_EQ("tab\ttab\ttab", json_decode("\"tab\\ttab\\ttab\""));
  ASSERT_EQ("quote\"quote\"end", json_decode("\"quote\\\"quote\\\"end\""));

  // Unicode escapes
  ASSERT_EQ("你好", json_decode("\"\\u4f60\\u597d\""));
  ASSERT_EQ("😄", json_decode("\"\\ud83d\\ude04\""));
  ASSERT_EQ("A", json_decode("\"\\u0041\""));
  ASSERT_EQ("©", json_decode("\"\\u00a9\""));

  // Mixed content
  ASSERT_EQ("Hello\n世界", json_decode("\"Hello\\n\\u4e16\\u754c\""));
}

void test_decode_booleans_null() {
  // Booleans
  ASSERT_EQ(1, json_decode("true"));
  ASSERT_EQ(0, json_decode("false"));
  ASSERT_EQ(1, json_decode(" true "));
  ASSERT_EQ(0, json_decode(" false "));

  // Null
  ASSERT_EQ(0, json_decode("null"));
  ASSERT_EQ(0, json_decode(" null "));
}

void test_decode_arrays() {
  // Empty array
  ASSERT_EQ(({}), json_decode("[]"));
  ASSERT_EQ(({}), json_decode(" [ ] "));

  // Simple arrays
  ASSERT_EQ(({1}), json_decode("[1]"));
  ASSERT_EQ(({1,2,3}), json_decode("[1,2,3]"));
  ASSERT_EQ(({1,2,3,4,5}), json_decode("[1,2,3,4,5]"));

  // Arrays with whitespace
  ASSERT_EQ(({1,2,3}), json_decode(" [ 1 , 2 , 3 ] "));
  ASSERT_EQ(({1,2,3}), json_decode("[\n1,\n2,\n3\n]"));

  // Arrays with mixed types
  ASSERT_EQ(({1,"hello",1,0}), json_decode("[1,\"hello\",true,false]"));
  ASSERT_EQ(({0,1,2,3,4,5,0,4,0.0}), json_decode("[0,1,2,3,4,5,-0,4,0.0]"));

  // Nested arrays
  ASSERT_EQ(({({})}), json_decode("[[]]"));
  ASSERT_EQ(({({1,2}),({3,4})}), json_decode("[[1,2],[3,4]]"));
  ASSERT_EQ(({({({1})})}), json_decode("[[[1]]]"));

  // Arrays with nulls
  ASSERT_EQ(({0,0,0}), json_decode("[null,null,null]"));
  ASSERT_EQ(({1,0,2}), json_decode("[1,null,2]"));
}

void test_decode_objects() {
  mapping result;

  // Empty object
  result = json_decode("{}");
  ASSERT_EQ(0, sizeof(result));

  result = json_decode(" { } ");
  ASSERT_EQ(0, sizeof(result));

  // Simple objects
  result = json_decode("{\"a\":1}");
  ASSERT_EQ(1, sizeof(result));
  ASSERT_EQ(1, result["a"]);

  result = json_decode("{\"name\":\"John\",\"age\":30}");
  ASSERT_EQ(2, sizeof(result));
  ASSERT_EQ("John", result["name"]);
  ASSERT_EQ(30, result["age"]);

  // Objects with whitespace
  result = json_decode(" { \"a\" : 1 , \"b\" : 2 } ");
  ASSERT_EQ(2, sizeof(result));
  ASSERT_EQ(1, result["a"]);
  ASSERT_EQ(2, result["b"]);

  // Objects with various value types
  result = json_decode("{\"int\":42,\"float\":3.14,\"string\":\"hello\",\"bool\":true,\"null\":null}");
  ASSERT_EQ(5, sizeof(result));
  ASSERT_EQ(42, result["int"]);
  ASSERT_EQ(3.14, result["float"]);
  ASSERT_EQ("hello", result["string"]);
  ASSERT_EQ(1, result["bool"]);
  ASSERT_EQ(0, result["null"]);

  // Nested objects
  result = json_decode("{\"outer\":{\"inner\":1}}");
  ASSERT_EQ(1, sizeof(result));
  ASSERT_EQ(1, result["outer"]["inner"]);

  result = json_decode("{\"person\":{\"name\":\"John\",\"address\":{\"city\":\"NYC\",\"zip\":10001}}}");
  ASSERT_EQ("John", result["person"]["name"]);
  ASSERT_EQ("NYC", result["person"]["address"]["city"]);
  ASSERT_EQ(10001, result["person"]["address"]["zip"]);

  // Objects with arrays
  result = json_decode("{\"numbers\":[1,2,3],\"names\":[\"a\",\"b\"]}");
  ASSERT_EQ(({1,2,3}), result["numbers"]);
  ASSERT_EQ(({"a","b"}), result["names"]);
}

void test_decode_complex_structures() {
  mapping result;

  // Mixed nested structures
  result = json_decode("{\"data\":[{\"id\":1,\"values\":[10,20]},{\"id\":2,\"values\":[30,40]}]}");
  ASSERT_EQ(2, sizeof(result["data"]));
  ASSERT_EQ(1, result["data"][0]["id"]);
  ASSERT_EQ(({10,20}), result["data"][0]["values"]);

  // Deep nesting
  result = json_decode("{\"a\":{\"b\":{\"c\":{\"d\":{\"e\":42}}}}}");
  ASSERT_EQ(42, result["a"]["b"]["c"]["d"]["e"]);
}

void test_encode_primitives() {
  // Integers
  ASSERT_EQ("0", json_encode(0));
  ASSERT_EQ("1", json_encode(1));
  ASSERT_EQ("42", json_encode(42));
  ASSERT_EQ("-1", json_encode(-1));
  ASSERT_EQ("-42", json_encode(-42));
  ASSERT_EQ("2147483647", json_encode(2147483647));

  // Floats
  ASSERT_EQ("0.000000", json_encode(0.0));
  ASSERT_EQ("1.500000", json_encode(1.5));
  ASSERT_EQ("3.141590", json_encode(3.14159));
  ASSERT_EQ("-1.500000", json_encode(-1.5));

  // Strings
  ASSERT_EQ("\"\"", json_encode(""));
  ASSERT_EQ("\"hello\"", json_encode("hello"));
  ASSERT_EQ("\"hello world\"", json_encode("hello world"));

  // Null/undefined
  ASSERT_EQ("null", json_encode(0));  // Note: 0 is used for null in LPC
}

void test_encode_string_escaping() {
  // Basic escapes
  ASSERT_EQ("\"\\\"\"", json_encode("\""));
  ASSERT_EQ("\"\\\\\"", json_encode("\\"));
  ASSERT_EQ("\"\\/\"", json_encode("/"));
  ASSERT_EQ("\"\\b\"", json_encode("\b"));
  ASSERT_EQ("\"\\f\"", json_encode("\x0c"));
  ASSERT_EQ("\"\\n\"", json_encode("\n"));
  ASSERT_EQ("\"\\r\"", json_encode("\r"));
  ASSERT_EQ("\"\\t\"", json_encode("\t"));

  // Special character (escape sequence)
  ASSERT_EQ("\"\\u001b\"", json_encode("\x1b"));
  ASSERT_EQ("\"\\u001b\"", json_encode("\e"));

  // Multiple escapes
  ASSERT_EQ("\"line1\\nline2\"", json_encode("line1\nline2"));
  ASSERT_EQ("\"quote\\\"end\"", json_encode("quote\"end"));
}

void test_encode_arrays() {
  // Empty array
  ASSERT_EQ("[]", json_encode(({})));

  // Simple arrays
  ASSERT_EQ("[1]", json_encode(({1})));
  ASSERT_EQ("[1,2,3]", json_encode(({1,2,3})));
  ASSERT_EQ("[1,2,3,4]", json_encode(({1,2,3,4})));

  // Arrays with mixed types
  ASSERT_EQ("[1,\"hello\",3.140000]", json_encode(({1,"hello",3.14})));

  // Nested arrays
  ASSERT_EQ("[[]]", json_encode(({({})})));
  ASSERT_EQ("[[1,2],[3,4]]", json_encode(({({1,2}),({3,4})})));
  ASSERT_EQ("[[[1]]]", json_encode(({({({1})})}))));
}

void test_encode_objects() {
  mapping m;
  string result;

  // Empty object
  ASSERT_EQ("{}", json_encode(([])));

  // Simple objects
  m = (["a":1]);
  ASSERT_EQ("{\"a\":1}", json_encode(m));

  m = (["name":"John","age":30]);
  result = json_encode(m);
  // Note: mapping key order may vary, so we check both contain expected substrings
  ASSERT(strsrch(result, "\"name\":\"John\"") != -1);
  ASSERT(strsrch(result, "\"age\":30") != -1);

  // Objects with various types
  m = (["int":42,"float":3.14,"string":"hello","bool":1]);
  result = json_encode(m);
  ASSERT(strsrch(result, "\"int\":42") != -1);
  ASSERT(strsrch(result, "\"string\":\"hello\"") != -1);

  // Nested objects
  m = (["outer":(["inner":1])]);
  ASSERT_EQ("{\"outer\":{\"inner\":1}}", json_encode(m));
}

void test_encode_non_string_keys() {
  mapping m;

  // Non-string keys should be skipped
  m = ([1:"value1", "key2":"value2", 3:"value3"]);
  ASSERT_EQ("{\"key2\":\"value2\"}", json_encode(m));

  // All non-string keys
  m = ([1:"a", 2:"b", 3:"c"]);
  ASSERT_EQ("{}", json_encode(m));
}

void test_encode_circular_references() {
  mapping m1, m2;
  mixed* arr1, arr2;

  // Circular mapping reference
  m1 = (["key":"value"]);
  m1["self"] = m1;
  ASSERT(strsrch(json_encode(m1), "\"self\":null") != -1);

  // Circular array reference
  arr1 = ({1,2,3});
  arr1 += ({arr1});
  ASSERT(strsrch(json_encode(arr1), "null") != -1);

  // Cross references
  m1 = (["name":"m1"]);
  m2 = (["name":"m2","ref":m1]);
  m1["ref"] = m2;
  // Should encode with null for circular part
  ASSERT(json_encode(m1));
}

void test_roundtrip() {
  mixed value, decoded;
  string encoded;

  // Integers
  value = 42;
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(value, decoded);

  // Floats
  value = 3.14;
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(value, decoded);

  // Strings
  value = "hello world";
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(value, decoded);

  // Arrays
  value = ({1,2,3,4,5});
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(value, decoded);

  // Nested arrays
  value = ({({1,2}),({3,4})});
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(value, decoded);

  // Objects
  value = (["name":"John","age":30]);
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ("John", decoded["name"]);
  ASSERT_EQ(30, decoded["age"]);

  // Complex nested structures
  value = ({1,"hello",({2,3}),(["key":"value"])});
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(4, sizeof(decoded));
  ASSERT_EQ(1, decoded[0]);
  ASSERT_EQ("hello", decoded[1]);

  // Special characters
  value = "\e你好";
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(value, decoded);

  value = "\x1b你好";
  encoded = json_encode(value);
  decoded = json_decode(encoded);
  ASSERT_EQ(value, decoded);
}

void test_real_world_json_files() {
  string content;

  // Test with real JSON file 1
  content = read_file("/single/tests/std/test.json");
  ASSERT(content);
  ASSERT(json_encode(json_decode(content)));

  // Test with real JSON file 2
  content = read_file("/single/tests/std/test2.json");
  ASSERT(content);
  ASSERT(json_encode(json_decode(content)));
}

void test_performance_large_arrays() {
  int start_time, end_time;
  mixed* large_array;
  string encoded;
  mixed decoded;

  // Build large array (1000 elements)
  large_array = allocate(1000);
  for(int i = 0; i < 1000; i++) {
    large_array[i] = i;
  }

  // Measure encoding time
  start_time = time_expression { encoded = json_encode(large_array); };
  write(sprintf("Large array (1000 elements) encoding: %d microseconds\n", start_time));

  // Measure decoding time
  start_time = time_expression { decoded = json_decode(encoded); };
  write(sprintf("Large array (1000 elements) decoding: %d microseconds\n", start_time));

  // Verify correctness
  ASSERT_EQ(1000, sizeof(decoded));
  ASSERT_EQ(0, decoded[0]);
  ASSERT_EQ(999, decoded[999]);
}

void test_performance_large_objects() {
  int start_time;
  mapping large_obj = ([]);
  string encoded;
  mixed decoded;

  // Build large object (500 keys)
  for(int i = 0; i < 500; i++) {
    large_obj[sprintf("key_%d", i)] = i;
  }

  // Measure encoding time
  start_time = time_expression { encoded = json_encode(large_obj); };
  write(sprintf("Large object (500 keys) encoding: %d microseconds\n", start_time));

  // Measure decoding time
  start_time = time_expression { decoded = json_decode(encoded); };
  write(sprintf("Large object (500 keys) decoding: %d microseconds\n", start_time));

  // Verify correctness
  ASSERT_EQ(500, sizeof(decoded));
  ASSERT_EQ(0, decoded["key_0"]);
  ASSERT_EQ(499, decoded["key_499"]);
}

void test_performance_deep_nesting() {
  int start_time;
  mapping deep_obj = ([]);
  mapping current = deep_obj;
  string encoded;
  mixed decoded;

  // Create deeply nested structure (50 levels)
  for(int i = 0; i < 50; i++) {
    current["level_" + i] = ([]);
    current = current["level_" + i];
  }
  current["value"] = 42;

  // Measure encoding time
  start_time = time_expression { encoded = json_encode(deep_obj); };
  write(sprintf("Deep nesting (50 levels) encoding: %d microseconds\n", start_time));

  // Measure decoding time
  start_time = time_expression { decoded = json_decode(encoded); };
  write(sprintf("Deep nesting (50 levels) decoding: %d microseconds\n", start_time));

  // Verify correctness
  current = decoded;
  for(int i = 0; i < 50; i++) {
    current = current["level_" + i];
  }
  ASSERT_EQ(42, current["value"]);
}

void test_performance_large_strings() {
  int start_time;
  string large_string = "";
  string encoded;
  mixed decoded;

  // Build large string (10000 characters)
  for(int i = 0; i < 1000; i++) {
    large_string += "0123456789";
  }

  // Measure encoding time
  start_time = time_expression { encoded = json_encode(large_string); };
  write(sprintf("Large string (10000 chars) encoding: %d microseconds\n", start_time));

  // Measure decoding time
  start_time = time_expression { decoded = json_decode(encoded); };
  write(sprintf("Large string (10000 chars) decoding: %d microseconds\n", start_time));

  // Verify correctness
  ASSERT_EQ(10000, strlen(decoded));
  ASSERT_EQ(large_string, decoded);
}

void do_tests() {
  if(!find_object("/std/json")) {
    write("json not loaded, skipped");
    return;
  }

  write("=== JSON Decode Tests ===\n");
  test_decode_numbers();
  write("✓ Number decoding tests passed\n");

  test_decode_strings();
  write("✓ String decoding tests passed\n");

  test_decode_booleans_null();
  write("✓ Boolean/null decoding tests passed\n");

  test_decode_arrays();
  write("✓ Array decoding tests passed\n");

  test_decode_objects();
  write("✓ Object decoding tests passed\n");

  test_decode_complex_structures();
  write("✓ Complex structure decoding tests passed\n");

  write("\n=== JSON Encode Tests ===\n");
  test_encode_primitives();
  write("✓ Primitive encoding tests passed\n");

  test_encode_string_escaping();
  write("✓ String escaping tests passed\n");

  test_encode_arrays();
  write("✓ Array encoding tests passed\n");

  test_encode_objects();
  write("✓ Object encoding tests passed\n");

  test_encode_non_string_keys();
  write("✓ Non-string key tests passed\n");

  test_encode_circular_references();
  write("✓ Circular reference tests passed\n");

  write("\n=== Round-trip Tests ===\n");
  test_roundtrip();
  write("✓ Round-trip tests passed\n");

  write("\n=== Real-world JSON Files ===\n");
  test_real_world_json_files();
  write("✓ Real-world JSON file tests passed\n");

  write("\n=== Performance Benchmarks ===\n");
  test_performance_large_arrays();
  test_performance_large_objects();
  test_performance_deep_nesting();
  test_performance_large_strings();
  write("✓ Performance benchmarks completed\n");

  write("\n=== All JSON tests passed! ===\n");
}
