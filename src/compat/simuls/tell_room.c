/*
 * Author: Matt Messier (Marius)
 * Description: An implementation of the old tell_room() efun
 */

#ifndef __NO_ENVIRONMENT__
varargs
void tell_room(mixed ob, mixed str, mixed exclude)
{
    if (stringp(ob))
        ob = find_object(ob);
    if (!ob || !objectp(ob))
        error("Bad argument 1 to tell_room()\n");
    exclude = (undefinedp(exclude) ? ({}) : (objectp(exclude) ? ({ exclude }) : exclude));
    if (!pointerp(exclude))
        error("Bad argument 3 to tell_room()\n");

    if (!(ob = environment(ob)))
        return;

    switch (typeof(str))
    {
        case "string":
            if (strlen(str) > __LARGEST_PRINTABLE_STRING__)
                error("Printable strings limited to length of " + __LARGEST_PRINTABLE_STRING__ + ".\n");
            break;

        case "float":  str += "";            break;
        case "int":    str += "";            break;
        case "object": str = file_name(str); break;

        default:
            error("Bad argument 2 to tell_room()\n");
    }

    foreach (ob in all_inventory(ob))
    {
        if (member_array(ob, exclude) == -1)
            tell_object(ob, str);
    }
}
#endif
