/*
 * Author: Matt Messier (Marius)
 * Description: Implementation of the old call_out mechanism.  The guts of the
 *              implementation resides in this object, but the simul_efun
 *              object is generally the only object that should call this
 *              directly.
 *
 * WARNING: valid_bind() in master.c MUST allow bind() to be called by this
 *          object in order for this to work.
 */

// function.h can be found in the driver include directory.
#include <function.h>

private inherit M_STATE;

class timer
{
    int    runtime;
    mixed  callback;
    mixed  args;
    object ob;
#ifdef THIS_PLAYER_IN_CALL_OUT
    object player;
#endif
}

private void run_timers(void);

private nosave int      timer_handle = 0;
private nosave int *    timer_order  = ({});
private nosave function timer_func   = (: run_timers :);
private nosave mapping  timer_list   = ([]);

protected
void create()
{
    restore_state();
}

protected
void destructor()
{
    save_state();
}

private
int timer_active(int handle)
{
    if (functionp(timer_list[handle]->callback))
    {
        if (functionp(timer_list[handle]->callback) & FP_OWNER_DESTED)
            return 0;
    }
    return (timer_list[handle]->ob != 0);
}

private
void prune_timers(void)
{
    timer_order = filter(timer_order, (: timer_active :));
    foreach (int handle in keys(timer_list) - timer_order)
        map_delete(timer_list, handle);
}

varargs
int set_timer(mixed callback, object ob, int delay, mixed args...)
{
    int         index, new_handle;
    class timer new_timer;

    if (!callback || (!stringp(callback) && !functionp(callback)))
        error("Bad argument 1 to set_timer");

    // Assign a new handle and make sure that we never assign -1 or 0
    if ((new_handle = ++timer_handle) == -2)
        timer_handle = 0;

    timer_list[new_handle] = new_timer = new(class timer);
    new_timer->runtime  = time() + (delay < 0 ? 0 : delay);
    new_timer->callback = callback;
    new_timer->args     = args;
    new_timer->ob       = ob;
#ifdef THIS_PLAYER_IN_CALL_OUT
    new_timer->player   = this_player();
#endif

    prune_timers();
    index = sizeof(timer_order);

    while (index--)
    {
        if (timer_list[timer_order[index]]->runtime <= new_timer->runtime)
        {
            timer_order = timer_order[0..index] + ({ new_handle }) + timer_order[index + 1..];
            return new_handle;
        }
    }

    timer_order = ({ new_handle }) + timer_order;
    if (sizeof(timer_order) == 1)
        master()->enable_pulse(timer_func);
    return new_handle;
}

mixed *query_timers(void)
{
    int   index, now;
    mixed func, *result;

    now = time();
    prune_timers();
    result = allocate(sizeof(timer_list));
    foreach (int handle, class timer data in timer_list)
    {
        if (functionp((func = data->callback)))
            func = "<function>";
        result[index++] = ({ data->ob, func, data->runtime - now });
    }
    return result;
}

int find_timer(mixed handle_or_name, object ob, int remove)
{
    foreach (int handle in timer_order)
    {
        if (timer_list[handle]->ob == ob &&
            ((intp(handle_or_name) && handle_or_name == handle) ||
             (stringp(handle_or_name) && handle_or_name == timer_list[handle]->callback)))
        {
            if (remove)
                timer_list[handle]->ob = 0;
            return timer_list[handle]->runtime - time();
        }
    }

    return -1;
}

int remove_timer(mixed handle_or_name, object ob)
{
    return find_timer(handle_or_name, ob, 1);
}

void remove_all_timers(object ob)
{
    foreach (int handle in timer_order)
    {
        if (timer_list[handle]->ob == ob)
            timer_list[handle]->ob = 0;
    }
    prune_timers();
}

private
void run_timers(void)
{
    function f;
    object   save_this_player;

    // Maybe save ourselves some time initially ...
    if (!sizeof(timer_order) || timer_list[timer_order[0]]->runtime > time())
        return;

    prune_timers();
    save_this_player = this_player();

    foreach (int handle in timer_order)
    {
        if (timer_list[handle]->runtime > time())
            break;
        if (!timer_active(handle))
            continue;

        if (stringp(timer_list[handle]->callback))
        {
            f = bind((: call_other, timer_list[handle]->ob,
                        timer_list[handle]->callback :),
                     timer_list[handle]->ob);
        }
        else
        {
            f = timer_list[handle]->callback;
        }
        timer_list[handle]->ob = 0;

#ifndef THIS_PLAYER_IN_CALL_OUT
        set_this_player(0);
#else
        set_this_player(timer_list[handle]->player);
#endif

        efun::reset_eval_cost();
        catch
        {
            if (undefinedp(timer_list[handle]->args))
                evaluate(f);
            else
                evaluate(f, timer_list[handle]->args);
        };
        efun::reset_eval_cost();
    }

    set_this_player(save_this_player);
}
