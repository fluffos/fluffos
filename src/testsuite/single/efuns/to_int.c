inherit "/inherit/tests";

int test(int which) {
    int i = 5;
    float f = 6.7;
    
    switch (which) {
    case 0:
	describe_test("float");
	i = to_int(f);
	return (i == 6);
    case 1:
	describe_test("implicit float");
	i = f;
	return (i == 6);
    case 2:
	describe_test("string");
	i = to_int("7");
	return (i == 7);
    case 3:
	describe_test("garbage string");
	i = to_int("ajdfj");
	return (i == 0);
    default:
	return -1;
    }
}
