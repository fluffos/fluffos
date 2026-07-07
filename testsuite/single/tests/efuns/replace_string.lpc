void do_tests() {
    string src = "This is a test";

    for (int i = 0; i < 2; i++) {
	ASSERT(replace_string(src, "", "bar") == src);
	ASSERT(replace_string(src, "foo", "bar", 5, 4) == src);
    
	// case 1: rlen <= plen, plen > 1, rlen > 0
	ASSERT(replace_string(src, "is", "x") == "Thx x a test");
	ASSERT(replace_string(src, "is", "x", 1) == "Thx is a test");
	ASSERT(replace_string(src, "is", "x", 0, 0) == "Thx x a test");
	ASSERT(replace_string(src, "is", "x", 0, 1) == "Thx is a test");
	ASSERT(replace_string(src, "is", "x", 0, 2) == "Thx x a test");
	ASSERT(replace_string(src, "is", "x", 1, 1) == "Thx is a test");
	ASSERT(replace_string(src, "is", "x", 1, 2) == "Thx x a test");
	ASSERT(replace_string(src, "is", "x", 2, 2) == "This x a test");
	// case 2: rlen <= plen, plen > 1, rlen == 0
        ASSERT(replace_string(src, "is", "") == "Th  a test");
	ASSERT(replace_string(src, "is", "", 1) == "Th is a test");
	ASSERT(replace_string(src, "is", "", 0, 0) == "Th  a test");
	ASSERT(replace_string(src, "is", "", 0, 1) == "Th is a test");
	ASSERT(replace_string(src, "is", "", 0, 2) == "Th  a test");
	ASSERT(replace_string(src, "is", "", 1, 1) == "Th is a test");
	ASSERT(replace_string(src, "is", "", 1, 2) == "Th  a test");
	ASSERT(replace_string(src, "is", "", 2, 2) == "This  a test");
        // case 3: rlen <= plen, plen == 1, rlen > 0
	ASSERT(replace_string(src, "s", "x") == "Thix ix a text");
	ASSERT(replace_string(src, "s", "x", 1) == "Thix is a test");
	ASSERT(replace_string(src, "s", "x", 2) == "Thix ix a test");
	ASSERT(replace_string(src, "s", "x", 0, 0) == "Thix ix a text");
	ASSERT(replace_string(src, "s", "x", 0, 1) == "Thix is a test");
	ASSERT(replace_string(src, "s", "x", 0, 2) == "Thix ix a test");
	ASSERT(replace_string(src, "s", "x", 1, 1) == "Thix is a test");
	ASSERT(replace_string(src, "s", "x", 1, 2) == "Thix ix a test");
	ASSERT(replace_string(src, "s", "x", 1, 3) == "Thix ix a text");
	ASSERT(replace_string(src, "s", "x", 2, 2) == "This ix a test");
	// case 4: rlen <= plen, plen == 1, rlen == 0
	ASSERT(replace_string(src, "s", "") == "Thi i a tet");
	ASSERT(replace_string(src, "s", "", 1) == "Thi is a test");
	ASSERT(replace_string(src, "s", "", 2) == "Thi i a test");
	ASSERT(replace_string(src, "s", "", 0, 0) == "Thi i a tet");
	ASSERT(replace_string(src, "s", "", 0, 1) == "Thi is a test");
	ASSERT(replace_string(src, "s", "", 0, 2) == "Thi i a test");
	ASSERT(replace_string(src, "s", "", 1, 1) == "Thi is a test");
	ASSERT(replace_string(src, "s", "", 1, 2) == "Thi i a test");
	ASSERT(replace_string(src, "s", "", 1, 3) == "Thi i a tet");
	ASSERT(replace_string(src, "s", "", 2, 2) == "This i a test");
	// case 5: rlen > plen, plen > 1, rlen > 0
        ASSERT(replace_string(src, "is", "foo") == "Thfoo foo a test");
	ASSERT(replace_string(src, "is", "foo", 1) == "Thfoo is a test");
	ASSERT(replace_string(src, "is", "foo", 2) == "Thfoo foo a test");
	ASSERT(replace_string(src, "is", "foo", 0, 0) == "Thfoo foo a test");
	ASSERT(replace_string(src, "is", "foo", 0, 1) == "Thfoo is a test");
	ASSERT(replace_string(src, "is", "foo", 0, 2) == "Thfoo foo a test");
	ASSERT(replace_string(src, "is", "foo", 1, 1) == "Thfoo is a test");
	ASSERT(replace_string(src, "is", "foo", 1, 2) == "Thfoo foo a test");
	ASSERT(replace_string(src, "is", "foo", 1, 3) == "Thfoo foo a test");
	ASSERT(replace_string(src, "is", "foo", 2, 2) == "This foo a test");
	// case 6: rlen > plen, plen == 1
        ASSERT(replace_string(src, "s", "foo") == "Thifoo ifoo a tefoot");
	ASSERT(replace_string(src, "s", "foo", 1) == "Thifoo is a test");
	ASSERT(replace_string(src, "s", "foo", 2) == "Thifoo ifoo a test");
	ASSERT(replace_string(src, "s", "foo", 0, 0) == "Thifoo ifoo a tefoot");
	ASSERT(replace_string(src, "s", "foo", 0, 1) == "Thifoo is a test");
	ASSERT(replace_string(src, "s", "foo", 0, 2) == "Thifoo ifoo a test");
	ASSERT(replace_string(src, "s", "foo", 1, 1) == "Thifoo is a test");
	ASSERT(replace_string(src, "s", "foo", 1, 2) == "Thifoo ifoo a test");
	ASSERT(replace_string(src, "s", "foo", 1, 3) == "Thifoo ifoo a tefoot");
	ASSERT(replace_string(src, "s", "foo", 2, 2) == "This ifoo a test");

	src = src[0..5] + src[6..];
    }
}
