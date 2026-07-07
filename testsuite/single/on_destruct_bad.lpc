void create()
{
    set_notify_destruct(1) ;
}

void on_destruct()
{
    int x, y ;
    rm("/data/test_on_destruct_bad") ;
    debug_message("on_destruct() called in " __FILE__) ;
    debug_message(sprintf("%O\n", x/y)) ;
}
