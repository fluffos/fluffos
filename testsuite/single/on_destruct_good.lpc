void create()
{
    set_notify_destruct(1) ;
}

void on_destruct()
{
    debug_message("on_destruct() called in " __FILE__) ;
    rm("/data/test_on_destruct_good") ;
}
