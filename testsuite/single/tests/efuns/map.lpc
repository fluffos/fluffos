int same(mixed* x, mixed* y) {
    if (sizeof(x) != sizeof(y)) return 0;
    for (int i = 0; i < sizeof(x); i++) {
	if (x[i] != y[i]) return 0;
    }
    return 1;
}

int msame(mapping x, mapping y) {
    mixed z;
    if (sizeof(keys(x)) != sizeof(keys(y))) return 0;
    foreach (z in keys(x)) {
	if (x[z] != y[z]) return 0;
    }
    return 1;
}

int whatever1(mixed x, mixed y, mixed z) {
    return x;
}

int whatever2(mixed x, mixed y, mixed z) {
    return y;
}

int whatever3(mixed x, mixed y, mixed z) {
    return z;
}

void do_tests() {
    /* array */
    ASSERT(same(map( ({ 1, 2, 0, 3 }), (: $1 :)), ({ 1, 2, 0, 3 })));
    ASSERT(map( ({ }), "foo") == ({}));
    ASSERT(same(map( ({ 2, 3 }), (: whatever1, 1 :)), ({ 1, 1 })));
    ASSERT(same(map( ({ 2, 3 }), "whatever2", this_object(), 1), ({ 1, 1})));
    ASSERT(same(map( ({ 2, 3 }), (: whatever2 :), 1), ({ 1, 1 })));
    ASSERT(same(map( ({ 2, 3 }), "whatever2", __FILE__, 1), ({ 1, 1 })));

    /* mapping */
    ASSERT(msame(map( ([ 1: 2, 0: 3 ]), (: $1 :)), ([ 1: 1, 0: 0 ])));
    ASSERT(msame(map( ([ ]), "foo"), ([])));
    ASSERT(msame(map( ([ 1: 2 ]), (: whatever1, 3 :)), ([ 1: 3 ])));
    ASSERT(msame(map( ([ 1: 2 ]), "whatever3", this_object(), 3), ([ 1: 3 ])));
    ASSERT(msame(map( ([ 1: 2 ]), (: whatever3 :), 3), ([ 1: 3 ])));
    ASSERT(msame(map( ([ 1: 2 ]), "whatever3", __FILE__, 3), ([ 1: 3 ])));

    /* string */
    ASSERT(map( "foobar", (: $1 + 1 :)) == "gppcbs");
    ASSERT(map("", "foo") == "");
    ASSERT(map( "xy", (: whatever1, 'c' :)) == "cc");
    ASSERT(map( "xy", "whatever2", this_object(), 'c') == "cc");
    ASSERT(map( "xy", (: whatever2 :), 'c')== "cc");
    ASSERT(map( "xy", "whatever2", __FILE__, 'c')== "cc");
}
