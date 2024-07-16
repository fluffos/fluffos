void do_tests()
{
    object ob ;

    ASSERT(ob = load_object("/single/on_destruct_good")) ;
    catch(destruct(ob)) ;
    ASSERT_EQ(0, objectp(ob)) ;

    catch(destruct(ob)) ;
    ASSERT_EQ(0, objectp(ob)) ;
}
