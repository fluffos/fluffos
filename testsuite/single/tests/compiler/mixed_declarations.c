// Test file for C99-style mixed declarations
// All tests should PASS (compile successfully)

void test_basic_mixed() {
    write("Test 1: Basic mixed declarations\n");
    int x = 1;
    write("x = " + x + "\n");
    string s = "hello";
    write("s = " + s + "\n");
}

void test_declaration_after_statement() {
    write("Test 2: Declaration after statement\n");
    write("Starting...\n");
    int y = 42;
    write("y = " + y + "\n");
}

void test_multiple_declarations_interspersed() {
    write("Test 3: Multiple declarations interspersed\n");
    int a = 1;
    write("a = " + a + "\n");
    int b = 2;
    write("b = " + b + "\n");
    a = a + b;
    int c = a * 2;
    write("c = " + c + "\n");
}

void test_nested_blocks() {
    write("Test 4: Nested blocks with mixed declarations\n");
    int outer = 1;
    write("outer = " + outer + "\n");
    {
        write("In inner block\n");
        int inner = 2;
        write("inner = " + inner + "\n");
        outer = outer + inner;
        int another = 3;
        write("another = " + another + "\n");
    }
    write("back in outer, outer = " + outer + "\n");
}

void test_loops_with_mixed_decls() {
    write("Test 5: Loops with mixed declarations\n");
    int count = 0;
    while(count < 3) {
        write("Loop iteration\n");
        int temp = count * 2;
        write("temp = " + temp + "\n");
        count++;
        int another = temp + 1;
        write("another = " + another + "\n");
    }
}

void test_for_loop_init_decl() {
    write("Test 6: For loop with init declaration\n");
    for(int i = 0; i < 3; i++) {
        write("i = " + i + "\n");
        int doubled = i * 2;
        write("doubled = " + doubled + "\n");
    }
}

void test_conditional_blocks() {
    write("Test 7: Conditional blocks with mixed declarations\n");
    int value = 5;
    if(value > 3) {
        write("In if block\n");
        int result = value * 2;
        write("result = " + result + "\n");
    }
    write("After if\n");
    int after = 10;
    write("after = " + after + "\n");
}

void test_switch_with_braced_cases() {
    write("Test 8: Switch with braced cases (should work)\n");
    int x = 1;
    switch(x) {
        case 1: {
            int local = 100;
            write("case 1: local = " + local + "\n");
            break;
        }
        case 2: {
            int other = 200;
            write("case 2: other = " + other + "\n");
            break;
        }
        default: {
            int def = 999;
            write("default: def = " + def + "\n");
            break;
        }
    }
}

void test_complex_expressions() {
    write("Test 9: Complex expressions with mixed declarations\n");
    int a = 5;
    int b = a + 3;
    write("b = " + b + "\n");
    int c = a * b;
    write("c = " + c + "\n");
    string msg = "result";
    write(msg + " = " + c + "\n");
}

void test_arrays_and_mappings() {
    write("Test 10: Arrays and mappings with mixed declarations\n");
    write("Creating array...\n");
    int *arr = ({ 1, 2, 3 });
    write("arr[0] = " + arr[0] + "\n");
    int first = arr[0];
    write("first = " + first + "\n");
    mapping m = ([ "key": 42 ]);
    int val = m["key"];
    write("val = " + val + "\n");
}

void test_function_calls_between_decls() {
    write("Test 11: Function calls between declarations\n");
    int x = 10;
    write("x = " + x + "\n");
    test_basic_mixed();  // call another function
    int y = 20;
    write("y = " + y + "\n");
}

void test_early_exit() {
    write("Test 12: Early return with mixed declarations\n");
    int x = 5;
    if(x > 3) {
        write("Early exit\n");
        int temp = 99;
        write("temp = " + temp + "\n");
        return;
    }
    int y = 10;
    write("This won't print\n");
}

void test_comma_expressions() {
    write("Test 13: Comma expressions with mixed declarations\n");
    int a = 1, b;
    b = 2;
    write("a = " + a + ", b = " + b + "\n");
    int c = (a = 5, b = 10, a + b);
    write("c = " + c + "\n");
}

void test_type_casting() {
    write("Test 14: Type casting with mixed declarations\n");
    int x = 42;
    write("x = " + x + "\n");
    string s = (string)x;
    write("s = " + s + "\n");
    int y = (int)"100";
    write("y = " + y + "\n");
}

void test_increment_operators() {
    write("Test 15: Increment operators with mixed declarations\n");
    int x = 5;
    write("x = " + x + "\n");
    x++;
    int y = ++x;
    write("y = " + y + ", x = " + x + "\n");
    int z = x--;
    write("z = " + z + ", x = " + x + "\n");
}

void test_scope_shadowing_prevention() {
    write("Test 16: Can't shadow variables (should still be enforced)\n");
    int x = 10;
    write("x = " + x + "\n");
    // This should still fail: int x = 20;
    // Uncomment to test that shadowing is still prevented
}

void test_all() {
    write("========================================\n");
    write("Running C99 Mixed Declaration Tests\n");
    write("========================================\n\n");
    
    test_basic_mixed();
    test_declaration_after_statement();
    test_multiple_declarations_interspersed();
    test_nested_blocks();
    test_loops_with_mixed_decls();
    test_for_loop_init_decl();
    test_conditional_blocks();
    test_switch_with_braced_cases();
    test_complex_expressions();
    test_arrays_and_mappings();
    test_function_calls_between_decls();
    test_early_exit();
    test_comma_expressions();
    test_type_casting();
    test_increment_operators();
    test_scope_shadowing_prevention();
    
    write("\n========================================\n");
    write("All tests completed!\n");
    write("========================================\n");
}

// Entry point for testing
void create() {
    test_all();
}
