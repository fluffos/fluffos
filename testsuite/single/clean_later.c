// clean_up() defined here

int clean_up(int inherited)
{
    message("shout", sprintf("CLEAN_UP() called in %O\n", this_object()), users()) ;
    return 1 ;
}
