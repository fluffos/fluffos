inherit "/inherit/tests";

int test(int which) {
    int i = 5;
    float f = 6.7;
    
    switch (which) {
    case 0:
	describe_test("int");
	f = to_float(i);
	return (f == 5.0);
    case 1:
	describe_test("implicit int");
	f = i;
	return (f == 5.0);
    case 2:
	describe_test("string");
	f = to_float("1.23");
	return (f == 1.23);
    case 3:
	describe_test("garbage string");
	f = to_float("ajdfj");
	return (f == 0.00);
    default:
	return -1;
    }
}
