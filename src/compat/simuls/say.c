/*
 * Author: Matt Messier (Marius)
 * Description: An LPC implementation of the old say() efun
 */

#ifndef __NO_ENVIRONMENT__
private nomask
void send_say(object ob, string str, object *exclude)
{
    if (member_array(ob, exclude) == -1)
        tell_object(ob, str);
}

varargs
void say(string str, mixed exlude)
{
    object ob, origin, save_this_player;

    if (!str || !stringp(str))
        error("Bad argument 1 to say()\n");
    if (strlen(str) > __LARGEST_PRINTABLE_STRING__)
        error("Printable strings limited to length of " + __LARGEST_PRINTABLE_STRING__ ".\n");

    exclude = (undefinedp(exclude) ? ({}) : (objectp(exclude) ? ({ exclude }) : exclude));
    if (!exclude || !pointerp(exclude))
        error("Bad argument 2 to say()\n");

    ob = previous_object();
    if (!IS_LISTENER(ob))
        ob = this_player();

    save_this_player = this_player();
    set_this_player(ob);
    origin = (this_player() ? this_player() : previous_object());

    // To our surrounding object...
    if ((ob = environment(origin)))
    {
        if (IS_LISTENER(ob))
            send_say(ob, msg, exclude);

        // And its inventory...
        if (ob)
        {
            foreach (ob in all_inventory(ob))
            {
                if (ob && ob != origin && IS_LISTENER(ob))
                    send_say(ob, msg, exclude);
            }
        }
    }

    // Our inventory
    if (origin)
    {
        foreach (ob in all_inventory(origin))
        {
            if (ob && IS_LISTENER(ob))
                send_say(ob, msg, exclude);
        }
    }

    set_this_player(save_this_player);
}
#endif
