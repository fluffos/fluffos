/*
 * Author: Matt Messier (Marius)
 * Description: Implementation of the heart_beat related efuns that have been
 *              removed from the driver:
 *
 *              heart_beats() - returns a list of all objects with heart_beats.
 *              query_heart_beat() - does an object have a heart_beat?
 *              set_heart_beat() - enable/disable an object's heart_beat
 */

varargs
int query_heart_beat(object ob)
{
    ob = (ob ? ob : (origin() != ORIGIN_LOCAL ? previous_object() : this_object()));
    return (int)HEART_BEAT_D->query_heart_beat_interval(ob);
}

void set_heart_beat(int val)
{
    object caller;

    caller = (origin() != ORIGIN_LOCAL ? previous_object() : this_object());
    HEART_BEAT_D->set_heart_beat_interval(caller, val);
}

object *heart_beats(void)
{
    return (object *)HEART_BEAT_D->query_heart_beat_list();
}
