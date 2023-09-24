inherit "/inherit/tests";

void do_tests() {
    ASSERT_EQ(""+abs(-1), "1");
    // integer -0 is not possible, changes to 0
    ASSERT_EQ(""+abs(0),  "0");
    ASSERT_EQ(""+abs(1),  "1");

    ASSERT_EQ(""+abs(-1.0), "1.000000");
    ASSERT_EQ(""+abs(-0.0), "0.000000");
    ASSERT_EQ(""+abs(0.0),  "0.000000");
    ASSERT_EQ(""+abs(1.0),  "1.000000");
}