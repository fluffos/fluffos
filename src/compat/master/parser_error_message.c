/*
 * Author: Tim Hollebeek (Beek)
 * Description: An example implementation of the parser_error_message() apply
 *              used if you use PACKAGE_PARSER.
 */

#ifdef __PACKAGE_PARSER__
string parser_error_message(int kind, object ob, mixed arg, int flag)
{
    string ret;

    ret = (ob ? (string)ob->short() + ": " : "");
    
    switch (kind)
    {
        case 1:
            if (flag)
                return ret + "There is no such " + arg + " here.\n";
            return ret + "There is no " + arg + " here.\n";

        case 2:
            if (flag)
                return ret + "None of the " + pluralize(arg) + " are alive.\n";
            return ret + "The " + arg + " isn't alive.\n";

        case 3:
            if (flag)
                return ret + "You can't reach them.\n";
            return ret + "You can't reach it.\n";

        case 4:
            {
                mixed  *descs = unique_array(arg, (: (string)$1->short() :));
                string str;

                if (sizeof(descs) == 1)
                    return ret + "Which " + (string)descs[0][0]->short() +
                           " do you mean?\n";

                str = ret + "Do you mean ";
                for (int i = 0;  i < sizeof(descs);  i++) {
                    if (sizeof(descs[i]) > 1)
                        str += "one of ";
                    str += (string)descs[i][0]->short() +
                           (i == sizeof(descs) - 1 ? " or " : ", ");
                }

                return str + "?\n";
            }

        case 5:
            if (arg > 1)
                return ret + "There are only " + arg + " of them.\n";
            return ret + "There is only one of them.\n";

        case 6:
            return ret + arg;

        case 7:
            return ret + "There is no " + arg + " here.\n";

        case 8:
            return ret + "You cannot use more than one object at a time with that verb.\n";
    }
}
#endif
