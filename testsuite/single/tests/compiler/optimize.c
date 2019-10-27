void bug(){
    string err = "", err2 = "test";

    ASSERT_EQ("\"\" \"test\"\n", sprintf("%O %O\n", err, err2));

    // Following two statements must be together for this bug to appear.
    err += err2;
    time_expression{
        ASSERT_EQ("\"test\" \"test\"\n", sprintf("%O %O\n", err, err2));
    };
    // No more statement below.
}

void bug2() {
    int cost, a, b;

    a = 0;
    b = 1;

    cost = time_expression{
        a = a + b;
    };
}
void do_tests() {
    bug();
    bug2();
}
