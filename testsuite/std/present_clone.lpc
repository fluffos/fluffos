object present_clone( mixed args... )
{
    object *found = ({ }) ;
    string source_file ;
    object source_ob ;
    object env ;
    int which ;

    if(!sizeof(args)) error("Missing argument 1 to present_clone.\n") ;

    if(stringp(args[0])) source_file = args[0] ;
    else if(objectp(args[0])) source_ob = args[0] ;
    else error("Invalid argument 1 passed to present_clone. Expected object or string, got " + typeof(args[0]) + ".\n") ;

    if(sizeof(args) >= 2)
    {
        if(intp(args[1])) which = args[1] ;
        else if(objectp(args[1])) env = args[1] ;
        else error("Invalid argument 2 passed to present_clone. Expected object or int, got " + typeof(args[0]) + ".\n") ;

        if(sizeof(args) == 3)
        {
            if(intp(args[2])) which = args[2] ;
            else error("Invalid argument 3 passed to present_clone. Expected int, got " + typeof(args[0]) + ".\n") ;

            if(sizeof(args) > 3)
            {
                error("Too many arguments passed to present_clone.\n") ;
            }
        }
        else
        {
            which = 0 ;
        }
    }
    else
    {
        env = previous_object() ;
        which = 0 ;
    }

    found = all_inventory( env ) ;
    if(!sizeof(found)) return 0 ;

    if(source_ob) source_file = base_name(source_ob) ;

    found = filter(found, function(object ob, string file) {
        return base_name(ob) == file ;
    }, source_file) ;

    if(which > sizeof(found) - 1 ) return 0 ;

    return found[which] ;
}
