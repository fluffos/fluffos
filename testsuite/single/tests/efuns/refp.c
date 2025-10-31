// Test file for refp() efun
// Tests that refp() correctly identifies reference parameters

void test_refp_with_ref_param(int ref value) {
    // Test that refp() returns 1 for reference parameters
    ASSERT_EQ(1, refp(value));

    // Modify the reference to ensure it's working
    value = 999;
}

void test_refp_with_normal_param(int value) {
    // Test that refp() returns 0 for normal parameters
    ASSERT_EQ(0, refp(value));
}

void test_refp_mixed_params(int ref ref_param, string normal_param, mapping ref map_param) {
    // Test mixed parameter types
    ASSERT_EQ(1, refp(ref_param));
    ASSERT_EQ(0, refp(normal_param));
    ASSERT_EQ(1, refp(map_param));

    // Verify we can modify the reference parameters
    ref_param = 123;
    map_param["test"] = "value";
}

void test_refp_array_ref(mixed ref *arr) {
    // Test refp with array reference
    ASSERT_EQ(1, refp(arr));

    // Modify the array to test reference behavior
    arr += ({ "new_element" });
}

void do_tests() {
    int x;
    string str;
    mapping m;
    mixed *array;

    x = 42;
    str = "hello";
    m = ([]);
    array = ({ 1, 2, 3 });

    // Test with reference parameter
    test_refp_with_ref_param(ref x);
    ASSERT_EQ(999, x);  // Verify the reference modification worked

    // Test with normal parameter
    test_refp_with_normal_param(x);
    ASSERT_EQ(999, x);  // Value should be unchanged

    // Test mixed parameters
    test_refp_mixed_params(ref x, str, ref m);
    ASSERT_EQ(123, x);  // Reference parameter was modified
    ASSERT_EQ("hello", str);  // Normal parameter unchanged
    ASSERT_EQ("value", m["test"]);  // Reference parameter was modified

    // Test array reference
    test_refp_array_ref(ref array);
    ASSERT_EQ(4, sizeof(array));  // Array should have new element
    ASSERT_EQ("new_element", array[3]);

    // Test that refp returns 0 for non-reference values passed directly
    ASSERT_EQ(0, refp(42));
    ASSERT_EQ(0, refp("string"));
    ASSERT_EQ(0, refp(({ 1, 2, 3 })));
    ASSERT_EQ(0, refp(([ "key": "value" ])));
}
