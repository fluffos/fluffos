/*
 * Author: Matt Messier (Marius)
 * Description: An LPC implementation of the old write() efun
 */

void write(mixed msg)
{
    object ob;

    ob = this_player();
#ifndef __NO_SHADOWS__
    if (!ob && shadow(previous_object(), 0))
        ob = previous_object();
    if (ob)
    {
        object s;

        while ((s = shadow(previous_object(), 0)))
            ob = s;
    }
#else
    if (!ob)
        ob = previous_object();
#endif

    switch (typeof(msg))
    {
        case "array":    msg = "<ARRAY>";                      break;
#ifndef __NO_BUFFER_TYPE__
        case "buffer":   msg = "<BUFFER>";                     break;
#endif
        case "float":    msg = msg + "";                       break;
        case "function": msg = "<FUNCTION>";                   break;
        case "int":      msg = msg + "";                       break;
        case "mapping":  msg = "<MAPPING>";                    break;
        case "object":   msg = "OBJ(" + file_name(msg) + ")";  break;
        case "string":                                         break;
        default:         msg = "<UNKNOWN>";                    break;
    }

    if (strlen(msg) > __LARGEST_PRINTABLE_STRING__)
        error("Printable strings limited to length of " + __LARGEST_PRINTABLE_STRING__ + ".\n");
    tell_object(ob, msg);
}
