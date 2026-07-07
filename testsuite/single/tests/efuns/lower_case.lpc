#define STR STR1
#define STR1 "this is a test!"
#define STR2 "This is a test!"
#define STR3 "This is A tEst!"

void do_tests() {
    string str;
    str = STR1;
    ASSERT(lower_case(str) == STR);
    ASSERT(str == STR1);

    str = STR2;
    ASSERT(lower_case(str) == STR);
    ASSERT(str == STR2);

    str = STR3;
    ASSERT(lower_case(str) == STR);
    ASSERT(str == STR3);
}
