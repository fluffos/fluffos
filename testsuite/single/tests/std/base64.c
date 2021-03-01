void do_tests()
{
    printf("base64encode: %s\n", base64encode("FluffOS is great!! :)")) ;
    ASSERT_EQ( base64encode("FluffOS is great!! :)"), "Rmx1ZmZPUyBpcyBncmVhdCEhIDop" ) ;
    printf("base64decode: %s\n", base64decode("Rmx1ZmZPUyBpcyBncmVhdCEhIDop")) ;
    ASSERT_EQ( base64decode("Rmx1ZmZPUyBpcyBncmVhdCEhIDop"), "FluffOS is great!! :)" ) ;
}
