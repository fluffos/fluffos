// Test NUMBER_STRING sefun

void do_tests()
{

    ASSERT_EQ( number_string( 0 ), "0" ) ;
    ASSERT_EQ( number_string( 100 ), "100" ) ;
    ASSERT_EQ( number_string( 1000 ), "1000" ) ;
    ASSERT_EQ( number_string( 123456789 ), "123456789" ) ;

    ASSERT_EQ( number_string( 0.0 ), "0.000000" ) ;
    ASSERT_EQ( number_string( 0.5 ), "0.500000" ) ;
    ASSERT_EQ( number_string( 100.5 ), "100.500000" ) ;
    ASSERT_EQ( number_string( 1000.6 ), "1000.600000" ) ;
    ASSERT_EQ( number_string( 123456789.123456 ), "123456789.123456" ) ;

    ASSERT_EQ( number_string( 0, 1 ), "0" ) ;
    ASSERT_EQ( number_string( 100, 1 ), "100" ) ;
    ASSERT_EQ( number_string( 1000, 1 ), "1,000" ) ;
    ASSERT_EQ( number_string( 123456789, 1 ), "123,456,789" ) ;

    ASSERT_EQ( number_string( 0.0, 1 ), "0.000000" ) ;
    ASSERT_EQ( number_string( 0.5, 1 ), "0.500000" ) ;
    ASSERT_EQ( number_string( 100.5, 1 ), "100.500000" ) ;
    ASSERT_EQ( number_string( 1000.6, 1 ), "1,000.600000" ) ;
    ASSERT_EQ( number_string( 123456789.123456, 1 ), "123,456,789.123456" ) ;

}
