void bug(){
    string err = "", err2 = "test";

    ASSERT_EQ("\"\" \"test\"\n", sprintf("%O %O\n", err, err2));

    // Follow two statement must be togehter for this bug to appear.
    err += err2;
    time_expression{
        ASSERT_EQ("\"test\" \"test\"\n", sprintf("%O %O\n", err, err2));
    };
    // No more statement below.
}
void do_tests() {
    bug();
}
