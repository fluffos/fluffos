/*
 * Author: Tim Hollebeek (Beek)
 * Description: Implementation of old efuns to call LPC code from a string
 *              process_value - parses a string and calls a function
 *              process_string - splits a string by @@ and passes each
 *                               resulting element to process_value
 */
mixed process_value(string func)
{
    int    tmp;
    object obj;
    string arg;

    if (!func || !stringp(func))
        return 0;

    if ((tmp = member_array('|', func)) != -1) {
        arg = func[tmp + 1..];
        func = func[0..tmp - 1];
    }
    if ((tmp = member_array(':', func)) != -1) {
        obj = find_object(func[tmp + 1..]);
        func = func[0..tmp - 1];
    } else {
        obj = previous_object();
    }

    if (arg)
        return (obj ? call_other(obj, func, explode(arg, "|")...) : 0);
    return (obj ? call_other(obj, func) : 0);
}

string process_string(string str)
{
    int    pr_start;
    string *parts = explode(str, "@@");
    
    pr_start = !(str[0..1]=="@@");
    
    for (int il = pr_start; il < sizeof(parts); il += 2) {
	string tmp = process_value(parts[il]);
	if (stringp(tmp))
	    parts[il] = tmp;
    }

    return implode(parts, "");
}
