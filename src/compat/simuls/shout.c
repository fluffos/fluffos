/*
 * Author: Matt Messier (Marius)
 * Description: An implementation of the old shout() efun
 */

// This is not a very good implementation, but if you're really using this efun
// unmodified in the first place, you ought to seriously rethink it.  Do you
// need to shout to monsters and everything, or just players?  If just players,
// change objects() to users() and optionally remove some of the extra checks.
void shout(string str)
{
    if (!str || !stringp(str))
        error("Bad argument 1 to shout()\n");
    if (strlen(str) > __LARGEST_PRINTABLE_STRING__)
        error("Printable strings limited to length of " + __LARGEST_PRINTABLE_STRING__ + ".\n");

    foreach (object ob in objects())
    {
        if (ob == this_player() || (!function_exists("catch_tell", ob) &&
            !function_exists("receive_message", ob)))
            continue;
#ifndef __NO_ENVIRONMENT__
        if (!environment(ob))
            continue;
#endif
        tell_object(ob, str);
    }
}
