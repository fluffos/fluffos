/*
 * Author: Matt Messier (Marius)
 * Description: Implementation of the old message() efun
 */

private nomask
void send_message(mixed type, string msg, mixed *to, mixed *exclude, int recurse)
{
    foreach (mixed ob in to)
    {
        if (stringp(ob))
            ob = find_object(ob);
        if (!ob || !objectp(ob))
            continue;

        if (!interactive(ob) && !function_exists("receive_message", ob))
        {
#ifndef __NO_ENVIRONMENT__
            if (recurse)
                send_message(type, msg, to, exclude, 0);
#endif
            continue;
        }

        if (member_array(ob, exclude) == -1)
            evaluate(bind((: $(ob)->receive_message($(type), $(msg)) :), ob));
    }
}

varargs
void message(mixed type, string msg, mixed to, mixed exclude)
{
    if (!msg || !stringp(msg))
        error("Bad argument 2 to message()\n");

    if (!to)    // for compatibility - boy is this ugly, sigh
    {
        if (strlen(msg) > __LARGEST_PRINTABLE_STRING__)
            error("Printable strings limited to length of " + __LARGEST_PRINTABLE_STRING__ + ".\n");
        // NOTE: Doesn't handle NONINTERACTIVE_STDERR_WRITE
        if (this_player())
            evaluate(bind((: receive, msg :), this_player()));
        return;
    }

    if (stringp(to))
        to = find_object(to);
    if (!to || (!objectp(to) && !pointerp(to)))
        error("Bad argument 3 to message()\n");
    to = (pointerp(to) ? to : ({ to }));

    exclude = (undefinedp(exclude) ? ({}) : (objectp(exclude) ? ({ exclude }) : exclude));
    if (!exclude || !pointerp(exclude))
        error("Bad argument 4 to message()\n");

    send_message(type, msg, to, exclude, 1);
}
