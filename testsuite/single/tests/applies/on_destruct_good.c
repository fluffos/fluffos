void create()
{
    set_notify_destruct(1) ;
}

void on_destruct()
{
    debug_message("on_destruct() called in " __FILE__) ;
}
