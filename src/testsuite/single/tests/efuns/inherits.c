#define OB2 "/single/tests/efuns/inh2.c"
#define OB1 "/single/tests/efuns/inh1.c"
#define OB0 "/single/tests/efuns/inh0.c"

void do_tests() {
    object ob2 = load_object(OB2);
    object ob1 = load_object(OB1);
    object ob = load_object(OB0);

    ASSERT(inherits(OB1, ob2));
    ASSERT(inherits(OB0, ob2));
    ASSERT(inherits(OB0, ob1));

    ASSERT(!inherits(OB2, ob1));
    ASSERT(!inherits(OB2, ob));
    ASSERT(!inherits(OB1, ob));
    ASSERT(!inherits("foo", ob));
}
