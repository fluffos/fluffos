void do_tests()
{
    object ob ;

    ASSERT(ob = load_object(__DIR__"on_destruct_good")) ;
    catch(destruct(ob)) ;
    ASSERT_EQ(0, objectp(ob)) ;

    ASSERT(ob = load_object(__DIR__"on_destruct_bad"))
    catch(destruct(ob)) ;
    ASSERT_EQ(0, objectp(ob)) ;
}
