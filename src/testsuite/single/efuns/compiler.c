inherit "/inherit/tests";

int test(int which) {
    mixed *code;
    
    switch (which) {
    case 0:
	describe_test("array constant optimizations: sizeof");
	code = __TREE__ ( sizeof( ({ 1, 3, 3 }) ) );
	return code[0] == "number" && code[1] == 3;
    case 1:
	describe_test("array constant optimizations: index");
	code = __TREE__ ( ({ 1, 3, 3 })[0] );
	return code[0] == "number" && code[1] == 1;
    case 2:
	describe_test("array constant optimizations: rindex");
	code = __TREE__ ( ({ 1, 3, 3 })[<2] );
	return code[0] == "number" && code[1] == 1;
    default:
	return -1;
    }
}
