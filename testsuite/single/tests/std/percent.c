// Test PERCENT sefun
// Test PERCENT_OF sefun

void do_tests()
{
    ASSERT_EQ( percent( 1, 5 ), 20 ) ;
    ASSERT_EQ( percent( 2.5, 50 ), 5.000000 ) ;

    ASSERT_EQ( percent_of( 25, 50 ), 12 ) ;
    ASSERT_EQ( percent_of( 25.0, 50 ), 12.500000 ) ;

}
