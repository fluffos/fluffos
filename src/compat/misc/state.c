/*
 * Author: Matt Messier (Marius)
 * Description: Used to save/restore state between objects -- use with
 *              modules/m_state
 * WARNING: This must not use any simul efuns since the simul efun object uses
 *          this to save its own state
 */

private nosave mapping objects = ([]);

private nomask
string base_file_name(object ob)
{
    int    i;
    string name;

    name = file_name(ob);
    i = strsrch(name, "#", -1);
    return (i == -1 ? name : name[0..i - 1]);
}

void save_data(string key, mixed data)
{
    string ob;

    ob = base_file_name(previous_object());
    if (!objects[ob])
        objects[ob] = ([]);
    objects[ob][key] = data;
}

mixed restore_data(string key)
{
    mixed  data;
    string ob;

    ob = base_file_name(previous_object());
    if (objects[ob])
    {
        data = objects[ob][key];
        map_delete(objects[ob], key);

        if (!sizeof(objects[ob]))
            map_delete(objects, ob);
    }

    return data;
}

void remove_object(void)
{
    string ob = base_file_name(previous_object());

    map_delete(objects, ob);
}
