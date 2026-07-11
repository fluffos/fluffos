
void do_tests() {
    ASSERT(implode( ({ 1, 2, 3 }), "foo") == "");
    ASSERT(implode( ({ "foo", "bar" }), "bazz") == "foobazzbar");
    ASSERT(implode( ({ 1, "foo", "bar" }), "bazz") == "foobazzbar");
    ASSERT(implode( ({ "foo", "bar" }), "") == "foobar");
    
    ASSERT(implode( ({ 1, 2, 3 }), (: $1 + $2 :)) == 6);
    ASSERT(implode( ({ 1, 2, 3 }), (: $1 + $2 :), "") == "123");
    ASSERT(implode( ({}), (: $1 :), 666) == 666);
    ASSERT(implode( ({}), (: $1 :)) == 0);
    ASSERT(implode( ({ 555 }), (: $1 + 5 :)) == 555);
}

