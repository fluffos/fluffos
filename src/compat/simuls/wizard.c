/*
 * Author: Matt Messier (Marius)
 * Description: Implementations of the enable_wizard(), disable_wizard(), and
 *              wizardp() efuns.
 *
 *              enable_wizard() - enable wizard status for the calling object
 *              disable_wizard() - disable wizard status for the calling object
 *              wizardp(ob) - does the object ob have wizard status?
 */

#ifndef NO_WIZARDS
private nosave object *wizard_list = ({});

void enable_wizard(void)
{
    int i;

    if (interactive(previous_object()))
    {
        if (member_array(previous_object(), wizard_list) == -1)
            if ((i = member_array(0, wizard_list)) == -1)
                wizard_list += ({ previous_object() });
            else
                wizard_list[i] = previous_object();
    }
}

void disable_wizard(void)
{
    int i;

    if (interactive(previous_object()))
    {
        if ((i = member_array(previous_object(), wizard_list)) != -1)
            wizard_list[i] = 0;
    }
}

int wizardp(object ob)
{
    if (ob && interactive(ob) && member_array(ob, wizard_list) != -1)
        return 1;
    return 0;
}
#endif
