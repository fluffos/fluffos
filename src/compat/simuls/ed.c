/*
 * Author: Matt Messier (Marius)
 * Description: Implementation of the old ed() efun.  Used only when OLD_ED is
 *              defined.  Also requires support in the interactive object.
 */

#ifdef OLD_ED
varargs
void ed(mixed arg1, mixed arg2, mixed arg3, mixed arg4)
{
    int    restricted;
    mixed  exitfn, writefn;
    object caller;
    string fname;

    if (!undefinedp(arg4))
    {
        if (!intp(arg4)) error("Bad argument 4 to ed()\n");

        if (!arg3 || (!stringp(arg3) && !functionp(arg3)))
            error("Bad argument 3 to ed()\n");
        if (!arg2 || (!stringp(arg2) && !functionp(arg2)))
            error("Bad argument 2 to ed()\n");
        if (!arg1 || !stringp(arg1))
            error("Bad argument 1 to ed()\n");

        fname      = arg1;
        exitfn     = arg2;
        writefn    = arg3;
        restricted = arg4;
    }
    else
    {
        if (!undefinedp(arg3))
        {
            if (intp(arg3))
                restricted = arg3;
            else if (!stringp(arg3) && !functionp(arg3))
                error("Bad argument 3 to ed()\n");
            else
                writefn = arg3;

            if (!arg2 || (!stringp(arg2) && !functionp(arg2)))
                error("Bad argument 2 to ed()\n");
            if (!arg1 || !stringp(arg1))
                error("Bad argument 1 to ed()\n");

            fname  = arg1;
            exitfn = arg2;
        }
        else
        {
            if (!undefinedp(arg2))
            {
                if (intp(arg2))
                    restricted = arg2;
                else if (!stringp(arg2) && !functionp(arg2))
                    error("Bad argument 2 to ed()\n");
                else
                    exitfn = arg2;

                if (!arg1 || !stringp(arg1))
                    error("Bad argument 1 to ed()\n");

                fname = arg1;
            }
            else
            {
                if (!undefinedp(arg1))
                {
                    if (intp(arg1)) restricted = arg1;
                    else if (!stringp(arg1)) error("Bad argument 1 to ed()\n");

                    fname = arg1;
                }
            }
        }
    }

#if defined(RESTRICTED_ED) && !defined(NO_WIZARDS)
    if (!wizardp(previous_object()))
        restricted = 1;
#endif

    caller = (origin() != ORIGIN_LOCAL ? previous_object() : this_object());
    if (stringp(exitfn))
        exitfn = bind((: call_other, caller, exitfn :), caller);

    caller->push_ed_session(fname, exitfn, writefn, restricted);
}
#endif
