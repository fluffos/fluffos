/*
 * Author: Matt Messier (Marius)
 * Description: Module to handle saving/restoring state -- used for object
 *              reloads -- depends on variables(), fetch_variable() and
 *              store_variable() -- all three in PACKAGE_CONTRIB
 */

void restore_state(void)
{
    foreach (string key in variables())
        store_variable(key, (mixed)STATE_D->restore_data(key));
    STATE_D->restore_sockets();
    STATE_D->remove_object();
}

void save_state(void)
{
    mixed *sockets;
    object ob;

    foreach (string key in variables())
        STATE_D->save_data(key, fetch_variable(key));
}
