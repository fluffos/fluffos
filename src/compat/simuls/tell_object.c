/*
 * Author: Matt Messier (Marius)
 * Description: An implementation of the old tell_object() efun
 */

void tell_object(object ob, string str)
{
    if (!ob)
    {
#ifdef __NONINTERACTIVE_STDERR_WRITE__
        // this assumes this object is not interactive
        // since this should be in the simul, that should be a safe
        // assumption to make here
        receive(str);
#endif
        return;
    }

#ifndef __INTERACTIVE_CATCH_TELL__
    if (interactive(ob))
    {
        if (strlen(str) > __LARGEST_PRINTABLE_STRING__)
            error("Printable strings limited to length of " + __LARGEST_PRINTABLE_STRING__ + ".\n");
        evaluate(bind((: receive, str :), ob));
        return;
    }
#endif

    evaluate(bind((: $(ob)->catch_tell($(str)) :), ob));
}
