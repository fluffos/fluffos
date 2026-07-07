int tmp = 0;

int third() {
    if (++tmp == 3) {
	tmp = 0;
	return 0;
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

int true() { return 1; }

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

void do_tests() {
    // array
    ASSERT(same(filter( ({ 1, 2, 0, 3 }), (: $1 :)), ({ 1, 2, 3 })));
    ASSERT(filter( ({}), "foo") == ({}));
    ASSERT(sizeof(filter( ({ 1 }), "true")));

    ASSERT(same(filter( ({ 1,2,3,4,5,6,7,8,9 }), "third", this_object())
	   , filter( ({ 1,2,3,4,5,6,7,8,9 }), (: third :))));

    ASSERT(sizeof(filter( ({ 0 }), (: whatever1, 1 :))));
    ASSERT(sizeof(filter( ({ 0 }), "whatever2", this_object(), 1)));
    ASSERT(sizeof(filter( ({ 0 }), (: whatever2 :), 1)));
    ASSERT(sizeof(filter( ({ 0 }), "whatever2", __FILE__, 1)));

    // mapping
    ASSERT(msame(filter( ([ 1 : 2, 0 : 3 ]), (: $1 :)), ([ 1 : 2 ])));
    ASSERT(msame(filter( ([]), "foo"), ([])));
    ASSERT(sizeof(filter( ([ 1 : 2 ]), "true")));

    ASSERT(msame(filter( ([ 1:2,3:4,5:6 ]), "third", this_object()),
		 filter( ([ 1:2,3:4,5:6 ]), (: third :))));

    ASSERT(!sizeof(filter( ([]), (: $1 :))));
    ASSERT(sizeof(filter( ([ 0 : 0 ]), (: whatever1, 1 :))));
    ASSERT(sizeof(filter( ([ 0 : 0 ]), "whatever3", this_object(), 1)));
    ASSERT(sizeof(filter( ([ 0 : 0 ]), (: whatever3 :), 1)));
    ASSERT(sizeof(filter( ([ 0 : 0 ]), "whatever3", __FILE__, 1)));
}
