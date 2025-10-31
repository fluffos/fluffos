// Test file for deref operator
// Tests that deref correctly dereferences reference parameters

void test_deref_basic(int ref value) {
    int original, new_value;

    // Test basic deref functionality
    original = deref value;
    ASSERT_EQ(42, original);

    // Modify through reference
    value = 100;

    // Verify deref gets the new value
    new_value = deref value;
    ASSERT_EQ(100, new_value);
}

void test_deref_string(string ref str) {
    string original, new_str;

    // Test deref with string reference
    original = deref str;
    ASSERT_EQ("hello", original);

    // Modify the string
    str = "world";

    // Verify deref gets the new value
    new_str = deref str;
    ASSERT_EQ("world", new_str);
}

void test_deref_array(mixed ref *arr) {
    mixed *original, *new_arr;

    // Test deref with array reference
    original = deref arr;
    ASSERT_EQ(3, sizeof(original));
    ASSERT_EQ(1, original[0]);
    ASSERT_EQ(2, original[1]);
    ASSERT_EQ(3, original[2]);

    // Modify the array
    arr = ({ "a", "b", "c" });

    // Verify deref gets the new array
    new_arr = deref arr;
    ASSERT_EQ(3, sizeof(new_arr));
    ASSERT_EQ("a", new_arr[0]);
    ASSERT_EQ("b", new_arr[1]);
    ASSERT_EQ("c", new_arr[2]);
}

void test_deref_mapping(mapping ref map) {
    mapping original, new_map;

    // Test deref with mapping reference
    original = deref map;
    ASSERT_EQ(2, sizeof(original));
    ASSERT_EQ("value1", original["key1"]);
    ASSERT_EQ("value2", original["key2"]);

    // Modify the mapping
    map = ([ "new": "mapping" ]);

    // Verify deref gets the new mapping
    new_map = deref map;
    ASSERT_EQ(1, sizeof(new_map));
    ASSERT_EQ("mapping", new_map["new"]);
}

void test_deref_in_expressions(int ref x, int ref y) {
    int sum;
    string result;

    // Test using deref in mathematical expressions
    sum = deref x + deref y;
    ASSERT_EQ(15, sum);  // 10 + 5

    // Test using deref in comparisons
    ASSERT(deref x > deref y);

    // Test using deref in function calls
    result = sprintf("x=%d, y=%d", deref x, deref y);
    ASSERT_EQ("x=10, y=5", result);
}

void test_deref_with_modification(int ref value) {
    int original, doubled;

    // Test that we can both deref and modify in same function
    original = deref value;
    value = original * 2;
    doubled = deref value;
    ASSERT_EQ(original * 2, doubled);
}

void mixed_ref_types_test(int ref int_ref, string ref str_ref,
                         mixed ref *arr_ref, mapping ref map_ref) {
    // Test deref on multiple reference types in one function
    ASSERT_EQ(123, deref int_ref);
    ASSERT_EQ("test", deref str_ref);
    ASSERT_EQ(2, sizeof(deref arr_ref));
    ASSERT_EQ("active", (deref map_ref)["status"]);

    // Modify all references
    int_ref = 456;
    str_ref = "modified";
    arr_ref = ({ "new", "array", "content" });
    map_ref = ([ "updated": "yes" ]);

    // Verify modifications via deref
    ASSERT_EQ(456, deref int_ref);
    ASSERT_EQ("modified", deref str_ref);
    ASSERT_EQ(3, sizeof(deref arr_ref));
    ASSERT_EQ("yes", (deref map_ref)["updated"]);
}

void do_tests() {
    int x, a, b, value, test_int;
    string str, test_str;
    mixed *arr, *test_arr;
    mapping map, test_map;

    x = 42;
    str = "hello";
    arr = ({ 1, 2, 3 });
    map = ([ "key1": "value1", "key2": "value2" ]);

    // Test basic deref functionality
    test_deref_basic(ref x);
    ASSERT_EQ(100, x);  // Verify reference modification worked

    // Test deref with string
    test_deref_string(ref str);
    ASSERT_EQ("world", str);  // Verify string reference modification

    // Test deref with array
    test_deref_array(ref arr);
    ASSERT_EQ(3, sizeof(arr));
    ASSERT_EQ("a", arr[0]);  // Verify array reference modification

    // Test deref with mapping
    test_deref_mapping(ref map);
    ASSERT_EQ(1, sizeof(map));
    ASSERT_EQ("mapping", map["new"]);  // Verify mapping reference modification

    // Test deref in expressions
    a = 10;
    b = 5;
    test_deref_in_expressions(ref a, ref b);

    // Test deref with modification in same function
    value = 50;
    test_deref_with_modification(ref value);
    ASSERT_EQ(100, value);  // 50 * 2

    // Test mixed reference types
    test_int = 123;
    test_str = "test";
    test_arr = ({ "item1", "item2" });
    test_map = ([ "status": "active" ]);

    mixed_ref_types_test(ref test_int, ref test_str, ref test_arr, ref test_map);

    // Verify all modifications occurred
    ASSERT_EQ(456, test_int);
    ASSERT_EQ("modified", test_str);
    ASSERT_EQ(3, sizeof(test_arr));
    ASSERT_EQ("content", test_arr[2]);
    ASSERT_EQ("yes", test_map["updated"]);
}
