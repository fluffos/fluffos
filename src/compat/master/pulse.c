/*
 * Author: Matt Messier (Marius)
 * Description: An example implementation of a pulse() apply in the master
 *              object, along with some added functions to use it.  This code
 *              is used by the simul code for call_out and heart_beat.
 */

private nosave mapping pulse_funcs = ([]);

void pulse(void)
{
    mapping new_list = ([]);

    foreach (object ob, function *funcs in pulse_funcs)
    {
        if (!ob || !pointerp(funcs))
            continue;

        foreach (function f in funcs)
        {
            if (f && !(functionp(f) & FP_OWNER_DESTED))
            {
                reset_eval_cost();
                catch( evaluate(f) );
            }
        }
    }

    //
    // Do this because when an object is a key in a mapping and that object is
    // destructed, the key/value pair become inaccessible.  This will someday
    // get corrected in MudOS.
    //
    reset_eval_cost();
    foreach (object ob, function *funcs in pulse_funcs)
    {
        if (ob)
            new_list[ob] = funcs;
    }
    pulse_funcs = new_list;
}

void enable_pulse(function f)
{
    object ob;

    if (!pulse_funcs[(ob = previous_object())])
    {
        pulse_funcs[ob] = ({ f });
    }
    else
    {
        if (member_array(f, pulse_funcs[ob]) == -1)
            pulse_funcs[ob] += ({ f });
    }
}

void disable_pulse(function f)
{
    object ob;

    if (pulse_funcs[(ob = previous_object())])
        pulse_funcs[ob] -= ({ f });
}
