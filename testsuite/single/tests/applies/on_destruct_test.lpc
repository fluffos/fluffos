void do_tests()
{
    object ob ;

    // Write a file that gets removed by the object when it gets destructed
    write_file("/data/test_on_destruct_good", "This is a test file.\n", 1) ;
    ASSERT_NE(-1, file_size("/data/test_on_destruct_good")) ;
    ASSERT(ob = load_object("/single/on_destruct_good")) ;
    catch(destruct(ob)) ;
    ASSERT_EQ(0, objectp(ob)) ;
    ASSERT_EQ(-1, file_size("/data/test_on_destruct_good")) ;

    // Write a file that gets removed by the object when it gets destructed
    write_file("/data/test_on_destruct_bad", "This is a test file.\n", 1) ;
    ASSERT(ob = load_object("/single/on_destruct_bad")) ;
    ASSERT_NE(-1, file_size("/data/test_on_destruct_bad")) ;
    catch(destruct(ob)) ;
    ASSERT_EQ(0, objectp(ob)) ;
    ASSERT_EQ(-1, file_size("/data/test_on_destruct_bad")) ;
}
