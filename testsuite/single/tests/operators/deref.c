void do_tests() {
    int x = 42;
    int *r = &x;  // Create a reference to x
    int y = deref r;  // Dereference the reference

    ASSERT(y == 42);

    // Test with different types
    string s = "hello";
    string *sr = &s;
    string result = deref sr;

    ASSERT(result == "hello");

    // Test with array
    int *arr = ({ 1, 2, 3 });
    int *ar = &arr;
    int *result_arr = deref ar;

    ASSERT(same(result_arr, ({ 1, 2, 3 })));
}
