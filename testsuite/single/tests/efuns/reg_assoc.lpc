int same_array(mixed *x, mixed *y) {
    if (!arrayp(x) || !arrayp(y)) return 0;
    if (sizeof(x) != sizeof(y)) return 0;
    for (int i = 0; i < sizeof(x); i++) {
	if (arrayp(x[i])) {
	    if (!same_array(x[i], y[i]))
		return 0;
	} else if (x[i] != y[i])
	    return 0;
    }
    return 1;
}

void do_tests() {
    ASSERT(same_array(
	       reg_assoc("testhahatest", ({ "haha", "te" }), ({ 2, 3 }), 4),
	       ({ ({ "", "te", "st", "haha", "", "te", "st" }),
		  ({ 4, 3, 4, 2, 4, 3, 4 }) })));
    ASSERT(catch(reg_assoc("foo", ({ 1 }), ({ 2, 3 }))));
    ASSERT(catch(reg_assoc("foo", ({ 1, 2 }), ({ 2, 3 }))));
    ASSERT(catch(reg_assoc("foo", ({ "bar", "+" }), ({ 0, 1 }))));
}
