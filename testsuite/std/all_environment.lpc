object *all_environment(object ob)
{
    object *result = ({ }) ;
    object env ;

    if(nullp(ob)) ob = previous_object() ;
    if(!objectp(ob)) error("Bad argument 1 to all_environment.\n") ;

    while(env = environment(ob)) { result += ({ env }) ; ob = env; }

    return result ;
}
