/*
 * Author: Matt Messier (Marius)
 * Description: Implementations of the call_out(), call_out_info(),
 *              find_call_out() and remove_call_out() efuns.
 *
 *              call_out() - set a new timer and callback
 *              call_out_info() - return a list of pending timers
 *              find_call_out() - return the time remaining for a timer
 *              remove_call_out() - delete a pending timer
 */

varargs
#ifdef CALLOUT_HANDLES
int
#else
void
#endif
call_out(mixed callback, int delay, mixed args...)
{
    object caller;

    if (!callback || (!stringp(callback) && !functionp(callback)))
        error("Bad argument 1 to call_out");

    caller = (origin() != ORIGIN_LOCAL ? previous_object() : this_object());

#ifdef CALLOUT_HANDLES
    return (int)CALL_OUT_D->set_timer(callback, caller, delay, args...);
#else
    CALL_OUT_D->set_timer(callback, caller, delay, args...);
#endif
}

mixed *call_out_info(void)
{
    return (mixed *)CALL_OUT_D->query_timers();
}

int find_call_out(mixed handle_or_name)
{
    object caller;

    caller = (origin() != ORIGINAL_LOCAL ? previous_object() : this_object());
#ifdef CALLOUT_HANDLES
    if (!intp(handle_or_name) && !stringp(handle_or_name))
        error("Bad argument 1 to find_call_out");
#else
    if (!handle_or_name || !stringp(handle_or_name))
        error("Bad argument 1 to find_call_out");
#endif
    return (int)CALL_OUT_D->find_timer(handle_or_name, caller, 0);
}

varargs
int remove_call_out(mixed handle_or_name)
{
    object caller;

    caller = (origin() != ORIGINAL_LOCAL ? previous_object() : this_object());
    if (undefinedp(handle_or_name))
    {
        CALL_OUT_D->remove_all_timers(caller);
        return 0;
    }

#ifdef CALLOUT_HANDLES
    if (!intp(handle_or_name) && !stringp(handle_or_name))
        error("Bad argument 1 to remove_call_out");
#else
    if (!handle_or_name || !stringp(handle_or_name))
        error("Bad argument 1 to remove_call_out");
#endif

    return (int)CALL_OUT_D->remove_timer(handle_or_name, caller);
}
