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
    mapping x = ([ 1 : "bazz" ]);
    
    ASSERT(catch(regexp("foo", "bar", x[1])));
    ASSERT(catch(regexp("foo", "bar", 1)));
    
    ASSERT(regexp("tabba", "a*b"));
    ASSERT(!regexp("tbba", "a+b"));
    ASSERT(same_array(
		      regexp( ({ "foo", "bar", "bazz" }), "b"),
		      ({ "bar", "bazz" })));
    ASSERT(same_array(
		      regexp( ({ "foo", "bar", "bazz" }), "^...$"),
		      ({ "foo", "bar" }) ));
    ASSERT(same_array(regexp( ({ "foo", "bar", "bazz" }), "(oo|zz)", 1),
		      ({ "foo", 1, "bazz", 3 })));
    ASSERT(catch(regexp("foo", "+")));
    ASSERT(catch(regexp( ({ "foo", "bar" }), "+")));
}
