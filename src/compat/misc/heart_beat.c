/*
 * Author: Matt Messier (Marius)
 * Description: Implementation of the old heart_beat mechanism
 */

private inherit M_STATE;

private void run_heart_beats(void);

private nosave function heart_beat_func = (: run_heart_beats :);
private nosave mapping  heart_beat_list = ([]);

object *query_heart_beat_list(void)
{
    return keys(heart_beat_list) - ({ 0 });
}

int query_heart_beat_interval(object ob)
{
    if (!heart_beat_list[ob])
        return 0;
    return heart_beat_list[ob][1];
}

void set_heart_beat_interval(object ob, int count)
{
    if (!count)
    {
        map_delete(heart_beat_list, ob);
        if (!sizeof(heart_beat_list))
            master()->disable_pulse(heart_beat_func);
        return;
    }

    if (!sizeof(heart_beat_list))
        master()->enable_pulse(heart_beat_func);

    if (heart_beat_list[ob])
        heart_beat_list[ob][1] = count;
    else
        heart_beat_list[ob] = ({ 0, count });
}

private
void run_heart_beats(void)
{
    int *   data;
    mapping new_list;
    object  ob;

    foreach (ob, data in heart_beat_list)
    {
        if (ob && ++data[0] == data[1])
        {
            data[0] = 0;
            reset_eval_cost();
            catch( evaluate(bind((: call_other, ob, "heart_beat" :), ob)) );
            reset_eval_cost();
        }
    }

    // filter out destructed objects - the hard way because mappings indexed
    // by objects are broken
    new_list = ([]);
    foreach (ob, data in heart_beat_list)
    {
        if (ob)
            new_list[ob] = data;
    }
    heart_beat_list = new_list;
}
